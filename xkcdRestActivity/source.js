  var myurl = 'http://dynamic.xkcd.com/api-0/jsonp/comic';
  $.ajax({
    url : myurl,
    dataType : "jsonp",
    success : function(parsed_json) {
      console.log(parsed_json);
      $("#xkcd").html("<img src="+JSON.stringify(parsed_json.img)+">");
    }
  });
