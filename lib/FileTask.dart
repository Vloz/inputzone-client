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
  
  int inputSize;
  
  
  FileTask(this.filename,this.input_file_url,this.inputSize){
    this.id = new DateTime.now().millisecondsSinceEpoch;
  }
  
  void start();
  
  void _postMessage(String message);
  
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
          output_file_url = m['url'];
          output_file_name = m['filename'];
          break;
        case MESSAGETYPE.DETAILS:
          details = message.body;
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
    status_id = int.parse(value);
    switch(STATUSTYPE.values[status_id]){
      case STATUSTYPE.CANCELED:
        status= 'Canceled';
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
        status= 'Finished, click the button to retrieve the converted file.';
        break;
      case STATUSTYPE.ERRRORED:
        status= 'ERROR';
        break;
      case STATUSTYPE.CANCELING:
              status= 'Canceling';
              break;
      default:
        status= 'unknown';
      
    }
  }
  
}





class EmscrTask extends FileTask{
  Worker _worker;
  
  EmscrTask(filename,url,size, this._worker):super(filename,url,size){
    _worker.onMessage.listen((MessageEvent m){
      var msg = new WorkerReceivedMessage.fromJSON(m.data);
      handleTaskMessage(new TaskMessage.FromEmscrMessage(UTF8.decode(msg.data))); 
    });
  }
  
  void start(){
    _postMessage('type\n'+MESSAGETYPE.START.toString()+'\nid\n'+id.toString()+'\nurl\n'+input_file_url
        +'\nfilename\n'+filename+'\nsize\n'+inputSize.toString()+'\nbrowser\n'+currentBrowser.toString());
  }
  
  void _postMessage(String message,{String funcName:'initWorker'}){   
    var data = [];
    UTF8.encode(message).forEach((e)=> data.add(e));
    _worker.postMessage(new WorkerPostMessage(funcName,id,data ));
  }
}




class PNaclTask extends FileTask{
  JsObject _pnaclProxy;
  
  PNaclTask(filename,url,size, this._pnaclProxy):super(filename,url,size);
  
  void start(){
      _postMessage('type\n'+MESSAGETYPE.START.toString()+'\nid\n'+id.toString()+'\nurl\n'+input_file_url+'\nfilename\n'+filename+'\nsize\n'+inputSize.toString());
    }
  
  void _postMessage(String message){
    _pnaclProxy.callMethod('postMessage', [message]);
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
}

class WorkerReceivedMessage{
  int callbackId;
  bool finalResponse;
  List data;
  
  WorkerReceivedMessage.fromJSON(Map o){
   //Map o = JSON.decode(json);
   callbackId = o["callbackId"];
   finalResponse = o["finalResponse"];
   data = o["data"];
  }
}