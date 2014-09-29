import 'dart:html';
import 'dart:async';
import 'package:polymer/polymer.dart';
import 'package:template_binding/template_binding.dart';
import '../../FileTask.dart';
import '../../iz_output_list.dart';


IzOutputList outputlist;

void main(){
  initPolymer().run(() {
                  Polymer.onReady.then((_) {  
                    var template = document.querySelector('#iz-body');
                    templateBind(template).model = new BodyModel();

                  });
                });
  window.navigator.temporaryStorage.requestQuota(1024*1024*1024*2,(_){
    ///TODO:Trouver pourquoi ce callback accepte pas l'utilisation de l'input
    print("Local file enabled!");
    },(e)=>window.alert("Cannot use your storage. Process aborted:"+e.message));

}


class BodyModel extends Observable {
  
  bool inited=false;
  
  void init(){
    inited =true;
    outputlist = querySelector('#outputlist');
  }
  
  void clickZone(){
    if(!inited)init();
    var task = new FileTask("test");
    outputlist.addTask(task);
//    InputElement input = new InputElement(type:'file');
//    input.onChange.first.then((evt)=>handleFile(evt));
//    input.click();
  }
  
  void handleFile(Event e){
     var input = (e.target as InputElement);
     var files = input.files;
     print(input.files.length.toString());

         // Loop through the FileList and render image files as thumbnails.
         input.files.forEach((f) {
           newInput(f);
           // Only process image files.
//          if (!f.type.match('image.*')) {
//            continue;
//          }
 //
//          var reader = new FileReader();
 //
//          // Closure to capture the file information.
//          reader.onLoad.listen((theFile) {
//            print(theFile.toString());
//          });
 //
//          // Read in the image file as a data URL.
//          reader.readAsArrayBuffer(f);
         });
   }
  
  
  void newInput(File file){
    var url = Url.createObjectUrl(file);
    var task = new FileTask(file.name);
    outputlist.addTask(task);
    //naclproxy.callMethod('postMessage', ['type\ncreate\nid\n'+task.id.toString()+'\nurl\n'+url+'\nfilename\n'+f.name+'\nsize\n'+f.size.toString()]); 
    
  }
  
 }

