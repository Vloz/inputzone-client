import 'package:polymer/polymer.dart';

class FileTask extends Observable{
  
  int id;
  
  String filename; 
  
  int progress = 0;
  
  String statut = 'running';
  
  FileTask(this.filename){
    this.id = new DateTime.now().millisecondsSinceEpoch;
    
  }
}