import 'package:polymer/polymer.dart';
import 'package:inputzone/inputzone.dart';
import 'dart:html';

@CustomTag('iz-output')
class IzOutput extends PolymerElement {
    
  @published FileTask context;
  
  IzOutput.created() : super.created(){  
  }
  
  void remove(){
    dispatchEvent(new CustomEvent('remove',detail: this.id ));
  }
  
  void ResumeORDownloadClicked(){
    if(STATUSTYPE.values[context.status_id]== STATUSTYPE.COMPLETED){
      iz_app.downloadTaskOutput(this.context);
    }
    else
      iz_app.querySpaceAndRun(context);
  }
  
  void CancelORDeleteClicked(){
    int s = STATUSTYPE.values[context.status_id];
    if( s == STATUSTYPE.COMPLETED || s == STATUSTYPE.ERRRORED || s == STATUSTYPE.CANCELED || s == STATUSTYPE.WAITINGQUOTA || s == STATUSTYPE.WAITINGUSERCLICK){
     context.deleteFsContent();
     context.remove();
    }else
      context.cancel();
  }
  
}