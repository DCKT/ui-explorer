const PORT = 4444

const express = require('express')
const app = express()
const fs = require('fs-promise')
const path = require('path')

app.set('view engine', 'pug')

const isFolder = (file) => !path.extname(file)

const resolveDirectories = (currentFolder) => (files) => {
  return Promise.all(files.map(file =>
    fs
      .lstat(path.resolve(currentFolder, file))
      .then(stats => ({
        name: file,
        isDirectory: stats.isDirectory()
      }))
  ))
}

app.get('/', (req, res) => {
  fs
    .readdir(__dirname)
    .then(resolveDirectories(__dirname))
    .then(files => {
      res.render('index', { files })
    })
    .catch(err => console.error(err))
})

app.get('/source/*', (req, res) => {
  const folderPathParams = req.path.replace('/source/', '')
  const folderPath = path.resolve(__dirname, folderPathParams)

  fs.stat(folderPath, function(err, stats) {
    const range = req.headers.range

    if (!range) {
      // 416 Wrong range
     return res.sendStatus(416)
    }
    const positions = range.replace(/bytes=/, "").split("-")
    const start = parseInt(positions[0], 10)
    const total = stats.size
    const end = positions[1] ? parseInt(positions[1], 10) : total - 1
    const chunksize = (end - start) + 1

    res.writeHead(206, {
      "Content-Range": `bytes ${start}-${end}/${total}`,
      "Accept-Ranges": "bytes",
      "Content-Length": chunksize,
      "Content-Type": "video/mp4"
    })

    const stream = fs
      .createReadStream(folderPath, {
        start,
        end
      })
      .on("open", () => stream.pipe(res))
      .on("error", err => res.end(err))
  })
})

app.get('*', (req, res) => {
  const folderPathParams = req.path.slice(1)
  const folderPath = path.resolve(__dirname, folderPathParams)

  if (isFolder(folderPath)) {
    fs
      .ensureFile(folderPath)
      .then(() => fs.readdir(folderPath))
      .then(resolveDirectories(folderPath))
      .then(files => {
        res.render('index', { files, folderPath: folderPathParams })
      })
      .catch(err => {
        console.error(err)
        res.render('error', { folderPath })
      })
  } else {
    res.render('details', { folderPath: folderPathParams })
  }
})

app.listen(PORT, () => console.log(`Server started on localhost:${PORT}`))
