part of InputZone;

class FileTask extends Observable{
  
  int id;
  
  String filename; 
  
  int progress = 0;
  
  String statut = 'running';
  
  FileTask(this.filename){
    this.id = new DateTime.now().millisecondsSinceEpoch;
    
  }
}

class MESSAGETYPE {
  final _value;
  const MESSAGETYPE._internal(this._value);
  toString() => 'Enum.$_value';

  static const START = const MESSAGETYPE._internal('1');
  static const CANCEL = const MESSAGETYPE._internal('2');
  static const PREPROGRESS = const MESSAGETYPE._internal('3');
  static const PROGRESS = const MESSAGETYPE._internal('4');
  static const ERROR = const MESSAGETYPE._internal('5');
  static const STATUS = const MESSAGETYPE._internal('6');
}