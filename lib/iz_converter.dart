import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'inputzone.dart';

@CustomTag('iz-converter')
class IzConverter extends PolymerElement {

  @published String title = "Title";
  @published String subtitle = "A short Subtitle"; 
  @published String tabname = "tabname";
  @published String inputExt="*/*";
  
  EmbedElement embed;
  JsObject naclproxy;
  
  ObservableList<FileTask> tasks = new ObservableList<FileTask>();

  
  IzConverter.created() : super.created(){ 
    embed= this.querySelector("embed");
    embed.onLoad.listen((_){
          embed.on['message'].listen((s){
            print("new message:"+(s as MessageEvent).data.toString());
            var msg = new TaskMessage((s as MessageEvent).data.toString());
            if(msg.id!='0')
              handleTaskMessage(msg);
          });
          naclproxy = new JsObject.fromBrowserObject(embed); 
        });
  }
  
  
  void clickZone(){
    InputElement input = new InputElement(type:'file')
    ..attributes.addAll({'accept':inputExt})
    ..onChange.first.then((evt)=>handleFile(evt))
    ..click();
  }
  

  void handleFile(Event e){
     var input = (e.target as InputElement);
     var files = input.files;

         // Loop through the FileList and render image files as thumbnails.
         input.files.forEach((f) {
           newInput(f);
         });
   }
  
  
  void newInput(File file){
    var url = Url.createObjectUrl(file);
    var task = new FileTask(file.name);
    addTask(task);

    naclproxy.callMethod('postMessage', ['type\n'+MESSAGETYPE.START.toString()+'\nid\n'+task.id+'\nurl\n'+url+'\nfilename\n'+file.name+'\nsize\n'+file.size.toString()]); 
  }
  
  void handleTaskMessage(TaskMessage message){
    FileTask targetTask = tasks.firstWhere((o)=>o.id == message.id);
    switch(message.messagetype){
      case MESSAGETYPE.PROGRESS:
        targetTask.updateProgress(message.body);
        break;
      case MESSAGETYPE.PREPROGRESS:
        targetTask.updateProgress(message.body,true);
        break;
      case MESSAGETYPE.STATUS:
        targetTask.updateStatus(message.body);
        break;
      case MESSAGETYPE.OUTPUTURL:
        targetTask.output_file_url = 'filesystem:http://'+window.location.host+message.body;
        break;
      case MESSAGETYPE.DETAILS:
        targetTask.details = message.body;
        break;
    }
    
  }
  
  
  void addTask(FileTask input){
    tasks.add(input);
  }
  
  void removeTask(Event e, var details, Node target){
    tasks.removeWhere((f)=> f.id == int.parse(details));
  }
  
  void cancelTask(Event e, var details, Node target){
    naclproxy.callMethod('postMessage', ['type\n'+MESSAGETYPE.CANCEL.toString()+'\nid\n'+details]); 
  }
  
}