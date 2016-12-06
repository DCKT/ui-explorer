const express = require('express')
const app = express()
const fs = require('fs-promise')
const path = require('path')

const __DEV__ = process.env.NODE_ENV !== 'production'
const PORT = 4444
const ROOT = __DEV__ ? __dirname : path.resolve(__dirname, '../downloads')

app.set('view engine', 'pug')
app.use(express.static('assets'))

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
    .readdir(ROOT)
    .then(resolveDirectories(ROOT))
    .then(files => {
      res.render('index', {
        files,
        isRoot: true
      })
    })
    .catch(err => console.error(err))
})

app.get('/source/*', (req, res) => {
  const folderPathParams = req.path.replace('/source/', '')
  const folderPath = path.resolve(ROOT, folderPathParams)

  fs.stat(folderPath, (err, stats) => {
    if (err) {
      return res.sendStatus(500)
    }

    const range = req.headers.range

    if (!range) {
      // 416 Wrong range
      return res.sendStatus(416)
    }
    const positions = range.replace(/bytes=/, '').split('-')
    const start = parseInt(positions[0], 10)
    const total = stats.size
    const end = positions[1] ? parseInt(positions[1], 10) : total - 1
    const chunksize = (end - start) + 1

    res.writeHead(206, {
      'Content-Range': `bytes ${start}-${end}/${total}`,
      'Accept-Ranges': 'bytes',
      'Content-Length': chunksize,
      'Content-Type': 'video/mp4'
    })

    const stream = fs
      .createReadStream(folderPath, {
        start,
        end
      })
      .on('open', () => stream.pipe(res))
      .on('error', err => res.end(err))
  })
})

app.get('/download/*', (req, res) => {
  const folderPathParams = req.path.replace('/download/', '')
  const folderPath = path.resolve(ROOT, folderPathParams)

  fs
    .stat(folderPath)
    .then(_ => {
      res.download(folderPath)
    })
    .catch(_ => res.sendStatus(500))
})

app.get('*', (req, res) => {
  const folderPathParams = req.path.slice(1)
  const folderPath = path.resolve(__dirname, folderPathParams)

  if (isFolder(folderPath)) {
    const currentPath = req.path.split('/')
    const backUrl = currentPath.slice(0, currentPath.length - 1).join('/') || '/'

    fs
      .ensureFile(folderPath)
      .then(() => fs.readdir(folderPath))
      .then(resolveDirectories(folderPath))
      .then(files => {
        res.render('index', {
          files,
          backUrl,
          folderPath: folderPathParams
        })
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
