import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'inputzone.dart';
import 'iz_app.dart';



@CustomTag('iz-converter')
class IzConverter extends PolymerElement {

  @published String title = "Title";
  @published String subtitle = "A short Subtitle"; 
  @published String tabname = "tabname";
  @published String inputExt="*/*";
  @published String inputHeader="";
  @published String outputHeader="";
  @published String credits="";
  @published String pnaclbin="";
  @published String emscrbin="";
  @observable bool initialized = false;
  @observable var runtime = RUNTIMETYPE.EMSCR;
  
  IzApp app;
  
  EmbedElement embed;
  JsObject pnaclProxy;
  
  Worker readyWorker;
  
  ObservableMap<int,FileTask> tasks = new ObservableMap<int,FileTask>();

  
  IzConverter.created() : super.created(){ 
    app = this.parent as IzApp;
    if(this.attributes.containsKey('active'))
      initConverter();
//    embed= this.querySelector("embed");
//    embed.onLoad.listen((_){
//          embed.on['message'].listen((s){
//            print("new message:"+(s as MessageEvent).data.toString());
//            var msg = new TaskMessage((s as MessageEvent).data.toString());
//            if(msg.id!='0')
//              handleTaskMessage(msg);
//          });
//          naclproxy = new JsObject.fromBrowserObject(embed); 
//        });
  }
  
  void ready(){
//    var data = [];
//    data = UTF8.encode("lol");
//        print(data);
//        var foo2 = [];
//        data.forEach((e)=>foo2.add(e));
//    
//     print(data.runtimeType.toString());
//     print([108, 111, 108].runtimeType.toString());
//     
//     List<int> foo = [108, 111, 108];
//     print(foo.runtimeType.toString());
//    w.postMessage(new workerMessage("workerReady",42,data));
  }
  
  void initConverter(){
    //window.navigator.mimeTypes.forEach((m)=>print(m.type));
    //print(); 
    if(!initialized && readyWorker==null && embed == null){
      if(app.runtime == RUNTIMETYPE.EMSCR){
        readyWorker = new Worker(emscrbin);
              readyWorker.onMessage.listen((MessageEvent e){
                initialized=true;
                readyWorker.terminate();
              });
              readyWorker.postMessage(new WorkerPostMessage("workerReady",0,[]));  
      } 
      else{
        embed= new EmbedElement()..type="application/x-pnacl"..src=pnaclbin..width='600'..height='100'..id='pnacl'..onLoad.listen((_){
          initialized=true;
              embed.on['message'].listen((s){
                print("new message:"+(s as MessageEvent).data.toString());
                var msg = new TaskMessage.FromNACLMessage((s as MessageEvent).data.toString());
                if(msg.id!=0){
                  tasks[msg.id].handleTaskMessage(msg);
                }
              });
              pnaclProxy = new JsObject.fromBrowserObject(embed); 
            });
        this.shadowRoot.children.add(embed);
         }
    }
   
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
    FileTask task;
    if(app.runtime == RUNTIMETYPE.PNACL){
      task = new PNaclTask(file.name, url, file.size, pnaclProxy);
      task.start();
    }else{
      task = new EmscrTask(file.name, url, file.size, new Worker(emscrbin));
      task.start();
    }
    addTask(task);

  }
  
  
  
  
  void addTask(FileTask input){
    tasks[input.id] = input;
  }
  
  void removeTask(Event e, var details, Node target){
    tasks.remove(tasks[int.parse(details)]);
  }
  
  void cancelTask(Event e, var details, Node target){
    pnaclProxy.callMethod('postMessage', ['type\n'+MESSAGETYPE.CANCEL.toString()+'\nid\n'+details]); 
  }
  
  

  
}


