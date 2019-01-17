// this is an embedded copy of update.htm that still works even when theres no spiffs, so a format of the spiffs
// doesn't knobble the entire device. Shouldn't be needed, but when updating from 2M flashed to a 4M board it is.
// try to keep this and update.htm looking about the same.
const char PROGMEM UPDATER[] = R"V0G0N(
<!doctype html><html><head><title>RFDesign TXMOD</title>
<style>
body {
    width: 80%;
    padding: 20px;
    background-color: #f1f1f1;
    font-family: Raleway;
    margin: 20px auto;
}
.doc { 
    background-color: #ffffff;
} 
.pale { 
    color:#ce8383;
    display: inline;
}
</style>
</head>

<body><div class='doc'><h1><a href='/'>RFDesign TXMOD</a></h1>

Your Module has TWO microcontrollers in it!<br>

<input type='file' name='spiffs' style="display:none" id="upload-spiffs" />
<input type='file' name='update' style="display:none" id="upload-firmware" /><br>
<input type='file' name='update' style="display:none" id="upload-rfdsik" /><br>
For Full System update: <div class='pale'>( Please do both, but only one of these at a time)</div><br>
Step 1:<button id="choose-firmware">Choose 'firmware.bin'</button> or something like 'RFDTxMod-V1.30.bin' <div class='pale'>( this flashes to the txmod )</div><br>
Step 2:<button id="choose-spiffs">Choose 'spiffs.bin'</button> or something like 'RFDTxMod-V1.30.spiffs.bin' <div class='pale'>(includes a standard RFDSiK900x.bin)</div><br>
<br>
To Just update the firmware in your 900x Radio:<br>
Optional:<button id="choose-rfdsik">Choose 'RFDSiK900x.bin'</button> or something like 'RFDSiK V2.65 rfd900x.bin'<br>

<hr>
Upload Progress:
<progress value="0" max="100" id="progressBar"></progress><br>
<hr>
<script>
// Show the file browse dialog
document.querySelector('#choose-spiffs').addEventListener('click', function() {
	document.querySelector('#upload-spiffs').click();
});
document.querySelector('#choose-firmware').addEventListener('click', function() {
	document.querySelector('#upload-firmware').click();
});
document.querySelector('#choose-rfdsik').addEventListener('click', function() {
	document.querySelector('#upload-rfdsik').click();
});

var which = -1;

function change_detector1() { which = 1; return change_detector();}
function change_detector2() { which = 2; return change_detector(); }
function change_detector3() { which = 3; return change_detector(); }

function change_detector() {
	// This is the file user has chosen

    var file;
    
	var file1 = document.querySelector('#upload-spiffs').files[0]; // this.files[0];
	var file2 = document.querySelector('#upload-firmware').files[0]; // this.files[0];
	var file3 = document.querySelector('#upload-rfdsik').files[0]; // this.files[0];

    if ( which == 1 ) { file = file1; action = "/update"; name = "spiffs"; filename = 'spiffs.bin'; 
                        filenamestart = 'RFDTxMod'; filenameend = '.spiffs.bin'; } 
    if ( which == 2 ) { file = file2; action = "/upload"; name = "firmware"; filename = 'firmware.bin'; 
                        filenamestart = 'RFDTxMo'; filenameend = '.bin';} 
    if ( which == 3 ) { file = file3; action = "/edit"; name = "update"; filename = 'RFDSiK900x.bin'; 
                        filenamestart = 'RFDSiK'; filenameend = '900x.bin';} 

	// Allowed types
	var mime_types = [ 'application/octet-stream' ]; // eg 'image/jpeg', 'image/png' ];
	
	// Validate MIME type
	if(mime_types.indexOf(file.type) == -1) {
		alert('Error : Incorrect file type');
		alert(file.type);
		return;
	}

	// Max 4 Mb allowed
	if(file.size > 4*1024*1024) {
		alert('Error : Exceeded size 4MB');
		return;
	}

    if (( file.name.startsWith(filenamestart) ) && ( file.name.endsWith(filenameend) ) && ( file.name != filename )  ) { 
        //alert("renaming file for upload");
        //file.name = filename; // cant just override file name as is readonly in the 'file' object.
       const newfile = new File([file], filename, {type: file.type});
       file=newfile;
    }
    
    if ( file.name != filename ) { 
	  alert('You tried to upload the incorrect file name!. Expecting: ' +filename+ " ( or something starting with '"+filenamestart+"') -> Got: "+file.name);
    }

    precache_success_page();
	
	up_file(name, action, file);
}

document.querySelector('#upload-spiffs').addEventListener('change', change_detector1);
document.querySelector('#upload-firmware').addEventListener('change', change_detector2);
document.querySelector('#upload-rfdsik').addEventListener('change', change_detector3);


var successtext = '';
function precache_success_page() { 
    var xhr = new XMLHttpRequest();
    xhr.responseType = 'text';

    xhr.onload = function () {
	    if (xhr.status >= 200 && xhr.status < 300) {
		    // Runs when the request is successful
		    //console.log(xhr.responseText);
	    }
        successtext = xhr.responseText;
        //alert(successtext);
    };

    xhr.open('GET', '/success.htm', true);

    xhr.send(null);
}

function up_file(name, action, file) { 
  var data = new FormData();
  
  var request = new XMLHttpRequest();
  
  // File selected by the user is called by what formname when POSTed? 
    data.append(name, file); 
  
  // AJAX request finished
  request.addEventListener('load', function(e) {
  	// request.response will hold the response from the server
  	//console.log(request.response);
  	//alert(request.response);

    var str = request.response;

    // SPIFFS:
    if  (str.search("Update Success") > 0 ) {  // from /update , means it's finished, and is already rebooting.
        // we can't redirect this type to another page after, as it's already rebooting...  
        //  we have preloaded a copy of /success.htm into a js object, and then we'll render that, if we can. 
        if ( successtext != '' ) { 
            document.open('text/html');
            document.write(successtext); // render /success.htm that we got earlier, becasue the hardware is rebooting right now.
            document.close(); 
        } else {  // fallback to a generic message.
            alert("Update Success! Rebooting...\nThis may take up-to a full MINUTE to come back on, so please be patient as it does this.");
        }
    // from /success.htm, means its finished and now NEEDS reboot
    } else if (str.search("File Upload Successful") > 0)  {  
        window.location.href = '/success.htm';
    } else { 
        // some error
        alert(request.response);
    }

  });
  
  // Upload progress on request.upload
  request.upload.addEventListener('progress', function(e) {
  	var percent_complete = (e.loaded / e.total)*100;
  	
  	// Percentage of upload completed
  	//console.log(percent_complete);
  	document.getElementById("progressBar").value = percent_complete;
  });
  
  // If server is sending a JSON response then set JSON response type
  request.responseType = 'text'; // or maybe 'json';
  
  // Send POST request to the server side script
  request.open('post', action); 
  request.send(data);
  
  // we handle the results in the 'load' event listener.

}

</script>



</body>
</html>

)V0G0N";
