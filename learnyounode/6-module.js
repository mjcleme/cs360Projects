var fs = require('fs');
var path = require('path');

module.exports = function(dirname, ext, callback) {
    fs.readdir(dirname, function(err, list) {
        if (err) {return callback(err);}
        var newList = [];
        for (var i = 0; i < list.length; i++) {
            if (path.extname(list[i]) === '.'+ext) {
                newList.push(list[i]);
            }
        }
        callback(null, newList);
    });
}
