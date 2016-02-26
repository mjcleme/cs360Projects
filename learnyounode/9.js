var http = require('http');

http.get(process.argv[2], function(response) {
    response.on('error', function (err) {console.log('Error');return;});
    response.setEncoding('utf8');
    var allData1 = '';
    response.on('data', function (data) {
        allData1 += data;
    });
    response.on('end', function() {
        http.get(process.argv[3], function(response2) {
            response2.on('error', function (err) {console.log('Error');return;});
            response2.setEncoding('utf8');
            var allData2 = '';
            response2.on('data', function (data) {
                allData2 += data;
            });
            response2.on('end', function() {
                http.get(process.argv[4], function(response3) {
                    response3.on('error', function (err) {
                        console.log('Error');return;
                    });
                    var allData3 = '';
                    response3.on('data', function (data) {
                        allData3 += data;
                    });
                    response3.on('end', function() {
                        console.log(allData1);
                        console.log(allData2);
                        console.log(allData3);
                    });
                });
            });
        });
    });
});
