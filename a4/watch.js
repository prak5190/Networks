var fs =  require('fs');
var child_process = require("child_process");
var runsync = require('runsync');
if(!child_process.spawnSync) {
  child_process.spawnSync = runsync.spawn;
}
function make() {
  child_process.spawnSync("make clean");
  var ret = child_process.spawnSync("make");
  if (ret.status == 0)
    return true ;
  else { 
    console.log(ret.output.join("\n"));
    return false;
  }
}

function create_peer(name , save_file,torrentFile,port) {
  var peerStr = "";
  var saveStr = "";
  if (port) 
    peerStr = " -p localhost:"+port + " " ;
  if (save_file)
    saveStr = " -s "+save_file + " ";
  var command = "./bt_client -v 5"+ peerStr + saveStr + torrentFile + " > ~/" + name + ".log";
  // var command = "./bt_client -v 4.2"+ peerStr + saveStr + "download.mp3.torrent > ~/" + name + ".log";
  console.log("COmmand " ,command);
  return child_process.exec(command, function (error, stdout, stderr) {
    //console.log('stdout: ' + stdout);
    if(stderr)
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
    var torrentFile = "download.mp3.torrent";
    create_peer("s1","s1.mp3",torrentFile);
    create_peer("s2","s2.mp3",torrentFile);
    for (var i = 3 ; i < 8 ; i++) {
      create_peer("s"+i ,"s"+i+".mp3",torrentFile);
    }
    // var torrentFile = "moby_dick.txt.torrent";
    // create_peer("s1","s1-moby.txt",torrentFile);
    // create_peer("s2","s2-moby.txt",torrentFile);
  }
}

run();
// Recursive just works in mac
fs.watch("./", { persistent : true , recursive : true }, function (event , fname) {
  //console.log("Event is : " , event , " , File  : " , fname);  
  if(/.c(pp)?$/.test(fname) || (/watch\.js/.test(fname))) {
    run();
  }
}); 
