var mymodule = require('./6-module.js');

mymodule(process.argv[2], process.argv[3], function(err, filteredList) {
    if (err) {console.log("Error");return;}
    filteredList.forEach(function(item) {
        console.log(item);
    });
});
