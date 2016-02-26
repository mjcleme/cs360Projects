var http = require('http');
var url = require('url');

var port = Number(process.argv[2]);

var server = http.createServer(function(req, res) {
    var parsedUrl = url.parse(req.url, true);
    var origTime = parsedUrl.query.iso;
    var endpoint = parsedUrl.pathname;
    var newTime = ''
    if (endpoint === '/api/parsetime') {
        var date = new Date(origTime);
        newTime = {
            'hour': date.getHours(),
            'minute': date.getMinutes(),
            'second': date.getSeconds()
        };
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(newTime));
    }
    else if (endpoint === '/api/unixtime') {
        var date = new Date(origTime);
        newTime = {'unixtime': date.getTime()};
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(newTime));
    }
    else {
        res.writeHead(404);
        res.end();
    }
});
server.listen(port);
