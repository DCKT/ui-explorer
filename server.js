const express = require("express");
const app = express();
const fs = require("fs-promise");
const path = require("path");
const cors = require("cors");
const __DEV__ = process.env.NODE_ENV !== "production";
const PORT = 4444;
const ROOT = __DEV__ ? __dirname : path.resolve(__dirname, "../downloads");

app.use(express.static("client/build"));

const isFolder = (file) => !path.extname(file);

const resolveDirectories = (currentFolder) => (files) => {
  return Promise.all(
    files.map((file) =>
      fs.lstat(path.resolve(currentFolder, file)).then((stats) => ({
        name: file,
        isDirectory: stats.isDirectory(),
      }))
    )
  );
};

app.get("/source/*", (req, res) => {
  const folderPathParams = decodeURIComponent(req.path.replace("/source/", ""));
  const folderPath = path.resolve(ROOT, folderPathParams);
  const fileExtension = path.extname(folderPath).slice(1);

  fs.stat(folderPath, (err, stats) => {
    if (err) {
      return res.sendStatus(500);
    }

    const range = req.headers.range;

    if (!range) {
      // 416 Wrong range
      return res.sendStatus(416);
    }
    const positions = range.replace(/bytes=/, "").split("-");
    const start = parseInt(positions[0], 10);
    const total = stats.size;
    const end = positions[1] ? parseInt(positions[1], 10) : total - 1;
    const chunksize = end - start + 1;

    res.writeHead(206, {
      "Content-Range": `bytes ${start}-${end}/${total}`,
      "Accept-Ranges": "bytes",
      "Content-Length": chunksize,
      "Content-Type": `video/${fileExtension}`,
    });

    const stream = fs
      .createReadStream(folderPath, {
        start,
        end,
      })
      .on("open", () => stream.pipe(res))
      .on("error", (err) => res.end(err));
  });
});

app.get("/download/*", (req, res) => {
  const folderPathParams = decodeURIComponent(
    req.path.replace("/download/", "")
  );
  const folderPath = path.resolve(ROOT, folderPathParams);

  fs.stat(folderPath)
    .then((stats) => {
      if (stats.isDirectory()) {
        const folderName = `${folderPath.split("/").reverse()[0]}.zip`;

        fs.readdir(folderPath, (err, files) => {
          if (err) {
            console.log(err);
            res.sendStatus(500);
          } else {
            res.zip(
              files.map((file) => {
                return {
                  path: `${folderPath}/${file}`,
                  name: file,
                };
              }),
              folderName
            );
          }
        });
      } else {
        res.download(folderPath);
      }
    })
    .catch((err) => {
      console.log("error", err);
      res.sendStatus(500);
    });
});

app.get("/api/*", cors(), (req, res) => {
  const requestPath = req.path.replace("/api/", "");
  const folderPathParams = decodeURIComponent(requestPath);
  const folderPath = path.resolve(ROOT, folderPathParams);

  if (isFolder(folderPath)) {
    const currentPath = requestPath.split("/");
    const backUrl =
      currentPath.slice(0, currentPath.length - 1).join("/") || "/";

    fs.readdir(folderPath)
      .then(resolveDirectories(folderPath))
      .then((files) => {
        const breadcrumb = requestPath.split("/");

        res.send({
          content: files,
          backUrl,
          folderPath: folderPathParams,
          breadcrumb: breadcrumb.map((path, index) => ({
            label: decodeURI(path),
            value: `/${breadcrumb.slice(0, index + 1).join("/")}`,
          })),
        });
      })
      .catch((err) => {
        console.error(err);
        res.render("error", { folderPath });
      });
  } else {
    res.render("details", {
      folderPath: folderPathParams,
      name: requestPath.split("/").slice(-1),
      breadcrumb: requestPath.slice(1).split("/"),
    });
  }
});

app.get("*", (req, res) => {
  res.render({
    fileName: "index.html",
  });
});

app.listen(PORT, () => console.log(`Server started on localhost:${PORT}`));
