part of InputZone;

class FileTask extends Observable{
  
  String id;
  
  @observable String filename='unknown'; 
  
  @observable String output_file_url=''; 
  
  @observable int progress = 0;
  
  @observable int preprogress = 0;
  
  @observable String status='unknown';  

  @observable int status_id=1;
  
  FileTask(this.filename){ 
    this.id = new DateTime.now().millisecondsSinceEpoch.toString();
  }
  
  void updateProgress(String value, [isPreprogress=false]){
    print("value:"+value);
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
        status= 'Finished, click the button to retrieve the converted file.';
        break;
      case STATUSTYPE.ERRRORED:
        status= 'ERROR';
        break;
      default:
        status= 'unknown';
      
    }
  }
  
}

class MESSAGETYPE {
  final _value;
  const MESSAGETYPE._internal(this._value);
  toString() => '$_value';

  static const NULL = const MESSAGETYPE._internal(0);
  static const START = const MESSAGETYPE._internal(1);
  static const CANCEL = const MESSAGETYPE._internal(2);
  static const PREPROGRESS = const MESSAGETYPE._internal(3);
  static const PROGRESS = const MESSAGETYPE._internal(4);
  static const ERROR = const MESSAGETYPE._internal(5);
  static const STATUS = const MESSAGETYPE._internal(6);
  static const OUTPUTURL = const MESSAGETYPE._internal(7);
  

  static get values => [NULL,START, CANCEL,PREPROGRESS,PROGRESS,ERROR,STATUS,OUTPUTURL];
}


class STATUSTYPE {
  final _value;
  const STATUSTYPE._internal(this._value);
  toString() => '$_value';

  static const CANCELED = const MESSAGETYPE._internal(0);
  static const STARTING = const MESSAGETYPE._internal(1);
  static const CONVERTING = const MESSAGETYPE._internal(2);
  static const COMPLETING = const MESSAGETYPE._internal(3);
  static const COMPLETED = const MESSAGETYPE._internal(4);
  static const ERRRORED = const MESSAGETYPE._internal(5);
  

  static get values => [CANCELED,STARTING, CONVERTING,COMPLETING,COMPLETED,ERRRORED];
}



class TaskMessage{
  String id='0';
  int messagetype;
  String body;
  
  TaskMessage(String naclMessage){
    var values = naclMessage.split('|');
    try{
      messagetype = MESSAGETYPE.values[int.parse(values[1])];   
      id=values.first; 
      body = values.last;
    }catch(e){
      body= naclMessage;
    }
  }
}