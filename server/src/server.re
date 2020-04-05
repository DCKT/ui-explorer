open Express;
open BsNode;
open Belt;

[@bs.val] external decodeUriComponent: string => string = "decodeURIComponent";
[@bs.val] external parseInt: ('a, int) => int = "parseInt";
[@bs.send]
external download: (Express.Response.t, string) => Express.complete =
  "download";

module Error: {
  type t;

  let lstat: t;
  let readdir: t;
  let toExn: t => exn;
} = {
  type t = string;

  let lstat = "lstat";
  let readdir = "readdir";

  external toExn: t => exn = "%identity";
};

let dirname: string = [%raw "__dirname"];
let isProduction = [%raw "process.env.NODE_ENV === 'production'"];
[@bs.val]
external productionDir: option(string) = "process.env.PRODUCTION_DIR";
let watchingFolder =
  switch (isProduction, productionDir) {
  | (true, None) => Js.Exn.raiseError("env PRODUCTION_DIR is missing")
  | (true, Some(v)) => Path.resolve([|dirname, v|])
  | _ => Path.resolve([|dirname, "../tests"|])
  };

let app = express();

type folderElement = {
  name: string,
  isDirectory: bool,
};

let checkElement = (~basePath, ~element) => {
  Js.Promise.make((~resolve, ~reject) => {
    Fs.lstat(Path.resolve([|basePath, element|]), (err, file) => {
      switch (err->Js.Nullable.toOption) {
      | None =>
        resolve(. {name: element, isDirectory: file->Fs.LStat.isDirectory()})
      | Some(err) =>
        Js.log2("LSTAT ERROR : \n", err);
        reject(. Error.(lstat->toExn));
      }
    })
  });
};

let scanDir = path =>
  Js.Promise.make((~resolve, ~reject) => {
    let basePath = Path.join([|watchingFolder, path|]);
    Fs.readdir(basePath, (err, elements) => {
      switch (err->Js.Nullable.toOption) {
      | None =>
        Js.Promise.all(
          elements->Array.map(element => checkElement(~basePath, ~element)),
        )
        |> Js.Promise.then_(results => {
             resolve(. results);
             Js.Promise.resolve();
           })
        |> ignore

      | Some(err) =>
        Js.log2("READDIR ERROR :\n", err);
        reject(. Error.(readdir->toExn));
      }
    });
  });

App.disable(app, ~name="x-powered-by");

App.get(app, ~path="/source/*") @@
Middleware.from((_, req, res) => {
  let sourcePath =
    req->Request.path
    |> Js.String.replace("/source/", "")
    |> decodeUriComponent;

  let filePath = Path.resolve([|watchingFolder, sourcePath|]);

  res |> Response.sendFile(filePath, None);
});

App.get(app, ~path="/download/*") @@
Middleware.from((_, req, res) => {
  let sourcePath =
    req->Request.path
    |> Js.String.replace("/download/", "")
    |> decodeUriComponent;

  let filePath = Path.resolve([|watchingFolder, sourcePath|]);

  res->download(filePath);
});

App.get(app, ~path="/*") @@
PromiseMiddleware.from((_, req, res) => {
  req
  |> Request.path
  |> decodeUriComponent
  |> scanDir
  |> Js.Promise.then_(elements => {
       res |> Response.sendJson(elements->Obj.magic) |> Js.Promise.resolve
     })
  |> Js.Promise.catch(err => {
       (
         switch (err->Obj.magic) {
         | e when e === Error.lstat => Response.sendString("lstat error", res)
         | e when e === Error.readdir =>
           Response.sendString("readdir error", res)
         | _ => Response.sendString("unknow error", res)
         }
       )
       |> Js.Promise.resolve
     })
});

App.listen(
  app,
  ~port=4444,
  ~onListen=
    e => {
      switch (e) {
      | exception (Js.Exn.Error(e)) =>
        Js.log(e);
        Node.Process.exit(1);
      | _ => Js.log @@ "Listening at http://127.0.0.1:4444"
      }
    },
  (),
);