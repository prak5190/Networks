var fs =  require('fs');
var child_process = require("child_process");
function make() {
  var ret = child_process.spawnSync("make");
  if (ret.status == 0)
    return true ;
  else { 
    console.log(ret.output.join("\n"));
    return false;
  }
}

function create_peer(name) {
  var command = "./bt_client moby_dick.txt.torrent > " + name + ".log";  
  console.log("COmmand " ,command);
  return child_process.exec(command, function (error, stdout, stderr) {
    //console.log('stdout: ' + stdout);
    console.log('stderr: ' + stderr);
    if (error !== null) {
      console.log('exec error: ' + error);
    }
  });
}

function kill_all() {
  return child_process.exec("pkill -f bt_client");
}

function run() {
  kill_all();
  if(make()) { 
    console.log("Triggering processes ");
    create_peer("s1");
    create_peer("s2");
  }
}

run();
// Recursive just works in mac
fs.watch("./", { persistent : true , recursive : true }, function (event , fname) {
  //console.log("Event is : " , event , " , File  : " , fname);  
  if(/.c(pp)?$/.test(fname)) {
    run();
  }
}); 
