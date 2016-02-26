var fs = require('fs');
var path = require('path');

fs.readdir(process.argv[2], function(err, list) {
    if (err) {console.log('Error');return;}
    var ext = process.argv[3];
    var newList = []
    for (var i = 0; i < list.length; i++) {
        if (path.extname(list[i]) === '.'+ext) {
            newList.push(list[i]);
        }
    }
    for (var i = 0; i < newList.length; i++) {
        console.log(newList[i]);
    }
});
