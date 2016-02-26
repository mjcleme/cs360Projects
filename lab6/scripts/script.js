var appId = 'a02j000000EaNOaAAN';
var redirectUri = 'http://ec2-54-148-248-229.us-west-2.compute.amazonaws.com/lab6';

var fslogin = function() {
    var familysearchUrl = 'https://sandbox.familysearch.org/cis-web/oauth2/v3/authorization?response_type=code&client_id=' + appId + '&redirect_uri=' + redirectUri;
    var familysearchWindow = window.open(familysearchUrl, '_self');
}

var getpersoninfo = function(event, accessToken) {
    event.preventDefault();
    var pid = $('#person-id-input').val();
    console.log(pid);
    $.ajax({
        url: 'https://sandbox.familysearch.org/platform/tree/persons/' + pid,
        headers: {'Accept': 'application/x-gedcomx-v1+json',
                  'Authorization': 'Bearer ' + event.data
                 },
        success: function(data) {
            console.log(data);
            $('#name').text('Name: ' + data['persons']['0']['display']['name']);
            $('#id').text('ID: ' + pid);
            $('#gender').text('Gender: ' + data['persons']['0']['display']['gender']);
            $('#birth').text('Birth Date: ' + data['persons']['0']['display']['birthDate']);
            $('#death').text('Death Date: ' + data['persons']['0']['display']['deathDate']);
        }
    });
}

$(document).ready(function() {
    var code = window.location.search.replace(/\?code=/, '');
    var accessToken;
    $(document.body).on('click', '#login-button', fslogin);
    if (code !== '') {
        console.log(code);
        $('#login-button').prop('disabled', true).addClass('disabled');
        $('#person-id-input').prop('disabled', false).removeClass('disabled');
        $('#person-id-submit').prop('disabled', false).removeClass('disabled');
        $.ajax({
            type: 'POST',
            url: 'https://sandbox.familysearch.org/cis-web/oauth2/v3/token',
            data: { 'grant_type': 'authorization_code',
                      'code': code,
                      'client_id': appId
                    },
            success: function(data) {
                console.log(data);
                accessToken = data['access_token'];
                console.log(accessToken);
                $(document.body).on('submit', '#id-input-form', accessToken, getpersoninfo);
            }
        });
    }
});
