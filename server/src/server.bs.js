// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Fs = require("fs");
var Path = require("path");
var Curry = require("bs-platform/lib/js/curry.js");
var Js_exn = require("bs-platform/lib/js/js_exn.js");
var Express = require("bs-express/src/Express.js");
var Process = require("process");
var Belt_Array = require("bs-platform/lib/js/belt_Array.js");
var Caml_js_exceptions = require("bs-platform/lib/js/caml_js_exceptions.js");

var lstat = "lstat";

var readdir = "readdir";

function Error_toExn(prim) {
  return prim;
}

var $$Error = {
  lstat: lstat,
  readdir: readdir,
  toExn: Error_toExn
};

var dirname = __dirname;

var isProduction = (process.env.NODE_ENV === 'production');

var match = process.env.PRODUCTION_DIR;

var watchingFolder = isProduction ? (
    match !== undefined ? Path.resolve(dirname, match) : Js_exn.raiseError("env PRODUCTION_DIR is missing")
  ) : Path.resolve(dirname, "../tests");

var app = Express.express(/* () */0);

function checkElement(basePath, element) {
  return new Promise((function (resolve, reject) {
                Fs.lstat(Path.resolve(basePath, element), (function (err, file) {
                        if (err == null) {
                          return resolve({
                                      name: element,
                                      isDirectory: file.isDirectory()
                                    });
                        } else {
                          console.log("LSTAT ERROR : \n", err);
                          return reject(Curry._1(Error_toExn, lstat));
                        }
                      }));
                return /* () */0;
              }));
}

function scanDir(path) {
  return new Promise((function (resolve, reject) {
                var basePath = Path.join(watchingFolder, path);
                Fs.readdir(basePath, (function (err, elements) {
                        if (err == null) {
                          Promise.all(Belt_Array.map(elements, (function (element) {
                                        return checkElement(basePath, element);
                                      }))).then((function (results) {
                                  resolve(results);
                                  return Promise.resolve(/* () */0);
                                }));
                          return /* () */0;
                        } else {
                          console.log("READDIR ERROR :\n", err);
                          return reject(Curry._1(Error_toExn, readdir));
                        }
                      }));
                return /* () */0;
              }));
}

Express.App.disable(app, "x-powered-by");

Express.App.get(app, "/source/*", Express.Middleware.from((function (param, req, res) {
            var sourcePath = decodeURIComponent(Express.$$Request.path(req).replace("/source/", ""));
            var filePath = Path.resolve(watchingFolder, sourcePath);
            return Express.$$Response.sendFile(filePath, undefined, res);
          })));

Express.App.get(app, "/download/*", Express.Middleware.from((function (param, req, res) {
            var sourcePath = decodeURIComponent(Express.$$Request.path(req).replace("/download/", ""));
            var filePath = Path.resolve(watchingFolder, sourcePath);
            return res.download(filePath);
          })));

Express.App.get(app, "/*", Express.PromiseMiddleware.from((function (param, req, res) {
            return scanDir(decodeURIComponent(Express.$$Request.path(req))).then((function (elements) {
                            return Promise.resolve(Express.$$Response.sendJson(elements, res));
                          })).catch((function (err) {
                          return Promise.resolve(err === lstat ? Express.$$Response.sendString("lstat error", res) : (
                                        err === readdir ? Express.$$Response.sendString("readdir error", res) : Express.$$Response.sendString("unknow error", res)
                                      ));
                        }));
          })));

Express.App.listen(app, 4444, undefined, (function (e) {
        var val;
        try {
          val = e;
        }
        catch (raw_exn){
          var exn = Caml_js_exceptions.internalToOCamlException(raw_exn);
          if (exn[0] === Js_exn.$$Error) {
            console.log(exn[1]);
            return Process.exit(1);
          } else {
            throw exn;
          }
        }
        console.log("Listening at http://127.0.0.1:4444");
        return /* () */0;
      }), /* () */0);

exports.$$Error = $$Error;
exports.dirname = dirname;
exports.isProduction = isProduction;
exports.watchingFolder = watchingFolder;
exports.app = app;
exports.checkElement = checkElement;
exports.scanDir = scanDir;
/* dirname Not a pure module */