var fs = require('fs');

var fileText = fs.readFileSync(process.argv[2]).toString();

console.log(fileText.split('\n').length-1);
