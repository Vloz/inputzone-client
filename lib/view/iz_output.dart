import 'package:polymer/polymer.dart';
import 'package:inputzone/inputzone.dart';
import 'dart:html';

@CustomTag('iz-output')
class IzOutput extends PolymerElement {
    
  @published FileTask context;
  @observable bool hasParams = false;
  bool consoleOpen = false;
  
  IzOutput.created() : super.created(){  
  }
  
  bool _ready=false;
  void ready(){
    if(_ready)
      return;
    _ready=true;
  }
  
  void remove(){
    dispatchEvent(new CustomEvent('remove',detail: this.id ));
  }
  
  void DownloadClicked(){
    if(STATUSTYPE.values[context.status_id]== STATUSTYPE.COMPLETED)
      iz_app.downloadTaskOutput(this.context);
      
  }
  
  void ResumeORCancelORDeleteClicked(){
    int s = STATUSTYPE.values[context.status_id];
    if( s == STATUSTYPE.WAITINGUSERCLICK){
      iz_app.querySpaceAndRun(context);
    }
    else if( s == STATUSTYPE.COMPLETED || s == STATUSTYPE.ERRRORED || s == STATUSTYPE.CANCELED || s == STATUSTYPE.WAITINGQUOTA){
     context.deleteFsContent();
     context.remove();
    }else
      context.cancel();
  }
  
  void SettingsClicked(){
    ($['console'] as TextAreaElement).style..height=(consoleOpen?"0":"auto")
        ..display= (consoleOpen?"none":"block");
    consoleOpen = !consoleOpen;
  }
  
}