open Js.Promise;

[@decco]
type breadcrumb = {
  label: string,
  value: string,
};

[@decco]
type response = {
  content: array(FolderContent.t),
  folderPath: string,
  backUrl: string,
  breadcrumb: array(breadcrumb),
};

let getFolder = path =>
  Axios.get("http://localhost:4444/api" ++ path)
  |> then_(res => res##data->resolve)
  |> then_(content =>
       switch (content->response_decode) {
       | Error(err) =>
         Js.log2("getFolder error", err);
         reject(Js.Exn.raiseError("decode error"));

       | Ok(v) => v->resolve
       }
     );