part of InputZone;

abstract class FileTask extends Observable{
  
  int id;
  
  @observable String filename='unknown'; 
  
  @observable String output_file_url=''; 
  
  String output_file_name=''; 
  
  String input_file_url=''; 
  
  @observable int progress = 0;
  
  @observable int preprogress = 0;
  
  @observable String status='unknown';  

  @observable int status_id=1;
  
  @observable String details=''; 
  
  @observable bool downloaded=false;
  
  int inputSize;
  
  int estimatedOutputSize;
  
  int taskSize=0; //could be estimate or the final outputsize
  
  int get estimatedSize=> inputSize+estimatedOutputSize;
  
  bool contentDeleted = false;
  
  
  FILESYSTEMTYPE fs_location;
  
  
  FileTask(this.filename,this.input_file_url,this.inputSize){
    this.id = new DateTime.now().millisecondsSinceEpoch;
  }
  
  void start(FILESYSTEMTYPE fileSystemType);
  void _postMessage(String message);
  void cancel();
  void deleteFsContent();
  
  void handleTaskMessage(TaskMessage message){
      switch(message.messagetype){
        case MESSAGETYPE.PROGRESS:
          updateProgress(message.body);
          break;
        case MESSAGETYPE.PREPROGRESS:
          updateProgress(message.body,true);
          break;
        case MESSAGETYPE.STATUS:
          updateStatus(message.body);
          break;
        case MESSAGETYPE.OUTPUTURL:
          Map m = JSON.decode(message.body);
          output_file_name = m['filename'];
          switch(fs_location){
            case FILESYSTEMTYPE.HTML5TEMP:
              Html5FS.temp().then((fs)=>fs.root.getFile(m['url']).then((e)=> e.getMetadata().then((m)=>updateTaskSize(m.size))));
              output_file_url = "filesystem:http://"+iz_app.appUrl.authority+"/temporary"+m['url'];
            break;
            case FILESYSTEMTYPE.HTML5PERS:
              Html5FS.pers().then((fs)=>fs.root.getFile(m['url']).then((e)=> e.getMetadata().then((m)=>updateTaskSize(m.size))));
              output_file_url = "filesystem:http://"+iz_app.appUrl.authority+"/persistent"+m['url'];
            break;
            default:
              output_file_url = m['url'];
          }
          break;
        case MESSAGETYPE.DETAILS:
          details = message.body;
          print(message.body);
          break;
        case MESSAGETYPE.ESTIMATESIZE:
          print(message.body);
          this.estimatedOutputSize = int.parse(message.body);
          iz_app.querySpaceAndRun(this);
          break;
        case MESSAGETYPE.ERROR:
          print(message.body);
          break;
      }
    }
  
  
  
  void updateProgress(String value, [isPreprogress=false]){
    if(isPreprogress)
      preprogress=int.parse(value); 
    else
      progress=int.parse(value); 
  }
  
  void updateStatus(String value){
    details='';
    status_id = int.parse(value);
    switch(STATUSTYPE.values[status_id]){
      case STATUSTYPE.CANCELED:
        status= 'Canceled';
        this.deleteFsContent();
        break;
      case STATUSTYPE.STARTING:
        status= 'Starting';
        break;
      case STATUSTYPE.CONVERTING:
        status= 'Converting';
        break;
      case STATUSTYPE.COMPLETING:
        status= 'Finalizing';
        break;
      case STATUSTYPE.COMPLETED:
        progress=100;//Sometime it stuck to 99%
        status= 'Finished, click the button to retrieve the file.';
        iz_app.showCompleteToast(this.id);
        break;
      case STATUSTYPE.ERRRORED:
        status= 'ERROR';
        this.deleteFsContent();
        break;
      case STATUSTYPE.CANCELING:
        status= 'Canceling';
        break;
      case STATUSTYPE.OPTIMIZINGRAM:
        status= 'Optimizing the RAM';
        break;
      case STATUSTYPE.WAITINGQUOTA:
        status= 'In queue, waiting space permssion';
        break;
      case STATUSTYPE.WAITINGUSERCLICK:
        status= 'Task paused, waiting manual click.';
        break;
      case STATUSTYPE.ESTIMATINGOUTPUTSIZE:
        status= 'Estimating output size';
        break;
      default:
        status= 'unknown';
      
    }
  }
  
  void updateTaskSize(int newValue){
    taskSize = newValue;
    if(fs_location==FILESYSTEMTYPE.HTML5TEMP)
      iz_app.getTempUsedSpace();
    else if(fs_location==FILESYSTEMTYPE.HTML5PERS)
      iz_app.getPersUsedSpace();
  }

  
  void remove(){
    iz_app.tasks.remove(this.id);
  }
    
  
}





class EmscrTask extends FileTask{
  Worker _worker;
  
  EmscrTask(filename,url,size, this._worker):super(filename,url,size){
    _worker.onMessage.listen((MessageEvent m){
      var msg = new WorkerReceivedMessage.fromJSON(m.data);
      handleTaskMessage(new TaskMessage.FromEmscrMessage(msg.message())); 
    });
  }
  
  void start(FILESYSTEMTYPE fileSystemType){
    fs_location = fileSystemType;
    updateTaskSize(estimatedSize);
    _postMessage('type\n'+MESSAGETYPE.START.toString()+'\nid\n'+id.toString()+'\nurl\n'+input_file_url
        +'\nfilename\n'+filename+'\nsize\n'+inputSize.toString()+'\nbrowser\n'+currentBrowser.toString()+'\nFS\n'+fileSystemType.toString());
  }
  
  void _postMessage(String message,{String funcName:'initWorker'}){   
    var data = [];
    UTF8.encode(message).forEach((e)=> data.add(e));
    _worker.postMessage(new WorkerPostMessage(funcName,id,data ));
  }
  
  void cancel(){
    _worker.terminate();
    updateStatus(STATUSTYPE.CANCELED.toString());
    
  }
  
  void deleteFsContent(){
    if(contentDeleted)
      return;
    
    contentDeleted = true;
    switch(fs_location){
      case FILESYSTEMTYPE.MEMFS:
        _worker.terminate(); 
        break;
      case FILESYSTEMTYPE.HTML5TEMP:
        Html5FS.temp().then((fs)=>fs.root.getDirectory('InputZone/'+this.id.toString()).then((e)=>e.removeRecursively()));
        taskSize=0;
        iz_app.getTempUsedSpace();
        break;
      case FILESYSTEMTYPE.HTML5PERS:
        Html5FS.pers().then((fs)=>fs.root.getDirectory('InputZone/'+this.id.toString()).then((e)=>e.removeRecursively()));
        taskSize=0;
        iz_app.getPersUsedSpace();
        break;
    }
    
  } 
  
}


class PNaclTask extends FileTask{
  JsObject _pnaclProxy;
  
  PNaclTask(filename,url,size, this._pnaclProxy):super(filename,url,size);
  
  void start(FILESYSTEMTYPE fileSystemType){
    fs_location = fileSystemType;
    updateTaskSize(estimatedSize);
      _postMessage('type\n'+MESSAGETYPE.START.toString()+'\nid\n'+id.toString()+'\nurl\n'+input_file_url+'\nfilename\n'
          +filename+'\nsize\n'+inputSize.toString()+'\nFS\n'+fileSystemType.toString());
    }
  
  void _postMessage(String message){
    _pnaclProxy.callMethod('postMessage', [message]);
  }
  
  void cancel(){
    updateStatus(STATUSTYPE.CANCELING.toString());
    _pnaclProxy.callMethod('postMessage', ['type\n'+MESSAGETYPE.CANCEL.toString()+'\nid\n'+id.toString()]); 
  }
  
  void deleteFsContent(){
      if(contentDeleted)
        return;
      
      contentDeleted = true;
      switch(fs_location){
        case FILESYSTEMTYPE.HTML5TEMP:
          Html5FS.temp().then((fs)=>fs.root.getDirectory('InputZone/'+this.id.toString()).then((e)=>e.removeRecursively()));
          break;
        case FILESYSTEMTYPE.HTML5PERS:
          Html5FS.pers().then((fs)=>fs.root.getDirectory('InputZone/'+this.id.toString()).then((e)=>e.removeRecursively()));
          break;
      }
    }
  
}




class TaskMessage{
  int id=0;
  int messagetype;
  String body;
  
  
  TaskMessage.FromNACLMessage(String naclMessage){
    var values = naclMessage.split('|');
    try{
      messagetype = MESSAGETYPE.values[int.parse(values[1])];   
      id=int.parse(values.first); 
      body = values.last;
    }catch(e){
      body= naclMessage;
    }
  }
  
  TaskMessage.FromEmscrMessage(String emscrMessage){
    var values = emscrMessage.split('|');
    try{
      messagetype = MESSAGETYPE.values[int.parse(values[1])];   
      //id=int.parse(values.first); 
      body = values.last;
    }catch(e){
      body= emscrMessage;
    }
  }
  
}


class WorkerPostMessage{
      String funcName;
      int callbackId;
      List data;
      
      WorkerPostMessage(this.funcName,this.callbackId,this.data);    
      
      WorkerPostMessage.fromString(this.funcName,this.callbackId,String data){
        var d = [];
        UTF8.encode(data.toString()).forEach((e)=> d.add(e));
        this.data = d;
      }
}

class WorkerReceivedMessage{
  int callbackId;
  bool finalResponse;
  List _data;
  
  WorkerReceivedMessage.fromJSON(Map o){
   callbackId = o["callbackId"];
   finalResponse = o["finalResponse"];
   _data = o["data"];
  }
  
  String message()=>UTF8.decode(_data);
  
  
}