import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'inputzone.dart';
import 'package:paper_elements/paper_toast.dart';


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
 
  
  EmbedElement embed;
  JsObject pnaclProxy;
  
  Worker infoWorker;
  
  ObservableMap<int,FileTask> tasks = new ObservableMap<int,FileTask>();

  
  IzConverter.created() : super.created(){ 
    if(this.attributes.containsKey('active'))
      initConverter();
  }
  
  void ready(){
//    var importDoc = document.currentScript.ownerDocument;
//    //var t = importDoc.querySelector('#disqus');
//    var clone = document.importNode(this.shadowRoot.querySelector('#disqus'), true);
//    //document.body.append(clone);
//    this.shadowRoot.append(clone);
//    
//    var dsq = document.createElement('script'); 
//    dsq.type = 'text/javascript'; 
//    dsq.async = true;
//    dsq.src = '//' + 'inputzone' + '.disqus.com/embed.js';
//    document.getElementsByTagName('body')[0].append(dsq);
  }
  
  void initConverter(){
    if(!initialized && infoWorker==null && embed == null){
      if(iz_app.runtime == RUNTIMETYPE.EMSCR){
        infoWorker = new Worker(emscrbin);
              infoWorker.onMessage.first.then((MessageEvent e){
                initialized=true;
                infoWorker.onMessage.listen(onEMSCREstimatedOutputSizeReceived);
              });
              infoWorker.postMessage(new WorkerPostMessage("workerReady",0,[]));  
      } 
      else{
        embed= new EmbedElement()..type="application/x-pnacl"..src=pnaclbin..width='0'..height='0'..id='pnacl'..onLoad.listen((_){
          initialized=true;
              embed.on['message'].listen((s){
                //print("new message:"+(s as MessageEvent).data.toString());
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
    if(iz_app.runtime == RUNTIMETYPE.PNACL)
      task = new PNaclTask(this,file.name, url, file.size, pnaclProxy);
    else
      task = new EmscrTask(this,file.name, url, file.size, new Worker(emscrbin));

    addTask(task);
    estimateOutputSize(task);
      

  }
  
  void addTask(FileTask input){
    tasks[input.id] = input;
  }
  
  void removeTask(Event e, var details, Node target){
    tasks.remove(tasks[int.parse(details)]);
  }
  
  void estimateOutputSize(FileTask task){
    if(iz_app.runtime == RUNTIMETYPE.EMSCR)
      infoWorker.postMessage(new WorkerPostMessage.fromString("estimateOutputSize",task.id,task.inputSize.toString()));
    else{
      pnaclProxy.callMethod('postMessage', ['type\n'+MESSAGETYPE.ESTIMATESIZE.toString()+'\nid\n'+task.id.toString()+'\ninputSize\n'+task.inputSize.toString()]);
    }
  }
  
  void onEMSCREstimatedOutputSizeReceived(MessageEvent msgEvent){
    var wmsg = new WorkerReceivedMessage.fromJSON(msgEvent.data);
    int id_target = wmsg.callbackId;
    int size = int.parse(wmsg.message());
    tasks[id_target].estimatedOutputSize = size;
    iz_app.querySpaceAndRun(tasks[id_target]);
  }
  
  void showCompleteToast(int id){
    (this.shadowRoot.querySelector('#pt'+id.toString()) as PaperToast).show();
  }
  
  void onPTDownloadClick(Event e, var detail, Node target){
    //print('id'+(e.target as DivElement).id);
    int id = int.parse((e.target as DivElement).id.substring(2));
    iz_app.downloadTaskOutput(tasks[id]);
  }

  
}


