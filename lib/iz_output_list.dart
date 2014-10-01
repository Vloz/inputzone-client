import 'package:polymer/polymer.dart';
import 'dart:html';
import 'inputzone.dart';

@CustomTag('iz-output-list')
class IzOutputList extends PolymerElement {

  
  ObservableList<FileTask> outputs1 = new ObservableList<FileTask>();
  
  //ObservableList<FileTask> outputs2 = new ObservableList<FileTask>();
  
  IzOutputList.created() : super.created(){  
  }
  
  void addTask(FileTask input){
    outputs1.add(input);
  }
  
  void removeTask(Event e, var details, Node target){
    print(details);
    outputs1.removeWhere((f)=> f.id == int.parse(details));
  }
  
  
}