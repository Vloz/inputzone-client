import 'package:polymer/polymer.dart';
import 'package:paper_elements/paper_dialog.dart';
import 'package:paper_elements/paper_tabs.dart';
import 'package:paper_elements/paper_input.dart';
import 'package:paper_elements/paper_tab.dart';
import 'package:core_elements/core_drawer_panel.dart';
import 'package:core_elements/core_animated_pages.dart';
import 'package:browser_detect/browser_detect.dart';
import 'dart:html';


import 'package:inputzone/iz_register.dart';
import 'package:inputzone/iz_converter.dart';
import 'inputzone.dart';

@CustomTag('iz-app')
class IzApp extends PolymerElement {
  
  @observable int availableTempSpace = 0;
  @observable int availablePersSpace = 0;
  @observable int uiUsedTemp=0; // just used for ui binding
  @observable int uiUsedPers=0; // just used for ui bindin
  @observable bool narrowedDrawer=false;
  @observable bool browserSupportPnacl=window.navigator.mimeTypes.any((mimetype)=> mimetype.type == 'application/x-pnacl');
  @observable bool browserSupportFS = false;
  @observable bool forcePersistent = false;
  
  List<IzConverter> _converters = new List<IzConverter>();
  FS_PERS_STATUS fs_pers_status = FS_PERS_STATUS.CLOSED;
  
  @observable ObservableList<FileTask> tasksWaitingOutOfQuotaDialog = new ObservableList<FileTask>();
  PaperDialog _outOfQuotaDialog;
  
  Uri appUrl;
  Map<String,String> options = {};
  @observable RUNTIMETYPE runtime=RUNTIMETYPE.EMSCR;

  
  IzApp.created() : super.created(){ 
    iz_app = this;
    detectBrowser();
    appUrl = Uri.parse(window.location.href); 
    if(appUrl.hasFragment){
      appUrl.fragment.split(";").forEach((f){
        if(f!=""){
          var opt = f.split("=");
          if(opt.length==2)
            options.addAll({opt[0].toLowerCase():opt[1].toLowerCase()});
          else
            options.addAll({opt[0].toLowerCase():''});
        } 
      });
    }
    
    if(options.containsKey('compatibilitymode')){
       runtime = RUNTIMETYPE.EMSCR;
     }
     else if(browserSupportPnacl){
       runtime = RUNTIMETYPE.PNACL;
       browserSupportPnacl=true;
       print("true");
     }     
    
    if(currentBrowser == BROWSER.CHROME){
      browserSupportFS = true;
      window.navigator.temporaryStorage.requestQuota(40*1024*1024*1024, (int received_size) {
        availableTempSpace = received_size;
            }, (e) {
              print(e);
              availableTempSpace =0;
            });
      
      window.onBeforeUnload.listen((e){    
        if(_converters.any((c)=>c.tasks.values.any((f)=>!f.contentDeleted))){
          Html5FS.temp().then((fs)=>fs.root.getDirectory('/InputZone').then((e)=> e.removeRecursively()));
          Html5FS.pers().then((fs)=>fs.root.getDirectory('/InputZone').then((e)=> e.removeRecursively()));
          e.returnValue="\n\nWARNING!\n\n You didn't purge every tasks before leaving.\n Next time, pay attention...\n\n";
        }
      });
    }

      generateTabs();
  }
  
  void ready(){
      _outOfQuotaDialog = $['outOfQuotaDialog'];
      tasksWaitingOutOfQuotaDialog.listChanges.listen((changes)=>
        changes.forEach((change){
          if(change.addedCount>0)
            if(!_outOfQuotaDialog.opened)
              _outOfQuotaDialog.toggle();
          if(change.removed.length>0)
            if(tasksWaitingOutOfQuotaDialog.length>0)
              _outOfQuotaDialog.toggle();
        })
      );
      var dsq = document.createElement('script'); 
      dsq.type = 'text/javascript'; 
      dsq.async = true;
      dsq.src = '//' + 'inputzone' + '.disqus.com/embed.js';
      document.getElementsByTagName('body')[0].append(dsq);
  }
  
  //Create header tabs depending on iz-converter content & select the "active" one
  void generateTabs(){
    int active =-1;
    if(options.containsKey('t'))
      active = int.parse(options['t']);
       if(this.querySelectorAll('iz-converter').length>1){
         this.querySelectorAll('iz-converter').forEach((converter){
           _converters.add(converter);
           
           PaperTab tabConverter = new Element.tag('paper-tab');
           tabConverter..attributes['role'] = 'tab'
               ..text = converter.id;
           ($['tabs'] as PaperTabs).children.add(tabConverter);
           int index = ($['tabs'] as PaperTabs).children.indexOf(tabConverter);
           
           if(index==active ||active==-1 && converter.attributes.keys.contains('active')){
             ($['tabs'] as PaperTabs).selected=index;
             ($['pages'] as CoreAnimatedPages).selected = index;
           }
           tabConverter.onClick.listen((_){
             int selIndex = ($['tabs'] as PaperTabs).children.indexOf(tabConverter).toInt();
             print(selIndex);
             setOption('t',selIndex.toString());
            // ($['pages'] as CoreAnimatedPages).selected = selIndex;
            // ((this.shadowRoot.querySelector('content') as ContentElement).getDistributedNodes()[selIndex] as IzConverter).initConverter();
           });
         });
       }
  }
  
   void querySpaceAndRun(FileTask taskToRun){
      if(currentBrowser == BROWSER.CHROME){
        if(!forcePersistent && taskToRun.estimatedSize<availableTempSpace-getTempUsedSpace())       
          taskToRun.start(FILESYSTEMTYPE.HTML5TEMP);     
        else{        
          switch(fs_pers_status){
            case FS_PERS_STATUS.CLOSED:
              fs_pers_status= FS_PERS_STATUS.OPENING;
              taskToRun.updateStatus(STATUSTYPE.WAITINGQUOTA.toString());
              window.navigator.persistentStorage.requestQuota(40*1024*1024*1024, 
                  (int received_size){
                     availablePersSpace = received_size;
                     fs_pers_status = FS_PERS_STATUS.OPENED;
                     _converters.forEach((IzConverter c)=>c.tasks.forEach((k,v){
                       if((v as FileTask).status_id.toString()==STATUSTYPE.WAITINGQUOTA.toString())
                         querySpaceAndRun(v);
                           })
                       );
                         }, (e) {
                           print(e); 
                           availableTempSpace =0;
                           }
                         );
              break;
            case FS_PERS_STATUS.OPENING:
              taskToRun.updateStatus(STATUSTYPE.WAITINGQUOTA.toString());
              break;
            case FS_PERS_STATUS.OPENED:
              if(availablePersSpace-getPersUsedSpace()>taskToRun.estimatedSize)
                taskToRun.start(FILESYSTEMTYPE.HTML5PERS);
              else{
                tasksWaitingOutOfQuotaDialog.add(taskToRun);
              }
          }
            
        }
        
      }else{
        ///TODO: Add space management for MEMFS
        taskToRun.start(FILESYSTEMTYPE.MEMFS);
      }
  }
  
   
   int getTempUsedSpace(){
     uiUsedTemp = _converters.fold(0, (a,b)=>a+b.tasks.values.where((f)=>f.fs_location == FILESYSTEMTYPE.HTML5TEMP).fold(0, (a,b)=>a+b.taskSize));
     return uiUsedTemp;
   }
   
   int getPersUsedSpace(){
     uiUsedPers = _converters.fold(0, (a,b)=>a+b.tasks.values.where((f)=>f.fs_location == FILESYSTEMTYPE.HTML5PERS).fold(0, (a,b)=>a+b.taskSize));
        return uiUsedPers;
      }
  
  void OOQD_Wait(){
    tasksWaitingOutOfQuotaDialog.last.updateStatus(STATUSTYPE.WAITINGUSERCLICK.toString());
    tasksWaitingOutOfQuotaDialog.remove(tasksWaitingOutOfQuotaDialog.last);
  }
  
  void OOQD_Force(){
      querySpaceAndRun(tasksWaitingOutOfQuotaDialog.last);
      tasksWaitingOutOfQuotaDialog.remove(tasksWaitingOutOfQuotaDialog.last);
    }
  
  void downloadTaskOutput(FileTask t){
    var a = new AnchorElement(href: t.output_file_url)..attributes.addAll({'Download':t.output_file_name});
          document.body.children.add(a);
          a.click();
          t.downloaded=true;
  }
   
  void onForcePersistentClick(){
    forcePersistent = !forcePersistent;
  }
  
  void onCompatiblityModeClick(){
    toggleOption('compatibilitymode');
    window.location.reload();
  }
  
  void setOption(String key, [String value='']){
    if(options.containsKey(key))
            options.remove(key);
    options.addAll({key:value}); 
    window.history.replaceState({}, '',appUrl.path+optionsToFragments());
  }
  
  void removeOption(String key){
      if(options.containsKey(key)){
        options.remove(key);
        window.history.replaceState({}, '',appUrl.path+optionsToFragments());
      }
    }
  
  void toggleOption(String key, [String value='']){
    if(options.containsKey(key))
      removeOption(key);
    else
      setOption(key, value);   
  }
  
  String optionsToFragments(){
    String fragments='';
        if(options.isNotEmpty)
           fragments= '#';
        options.forEach((k,v){
          fragments+=k;
          if(v!= '')
            fragments+='='+v;
          fragments+=';';
        });
        return fragments;
  }
  
  void onPurgeEverythingClick(){
    _converters.forEach((c)=>c.tasks.forEach((k,v)=>v.cancel()));
    Html5FS.temp().then((fs)=>fs.root.getDirectory('/InputZone').then((e)=> e.removeRecursively()));
    Html5FS.pers().then((fs)=>fs.root.getDirectory('/InputZone').then((e)=> e.removeRecursively()));
  }
  
  void onAboutClick(){
    ($['AboutDialog'] as PaperDialog).toggle();
  }
  
  void onSearchInput(Event e, var details, Node target){
    ($['register'] as IzRegister).filter((target as PaperInput).inputValue);
  }
  
  void onLogoClick(){
    window.location.href = "/";
  }
  
  void expandDrawer(){
    ($['drawer'] as CoreDrawerPanel).togglePanel();
  }
  
  void detectBrowser(){
    if(browser.isChrome)
      currentBrowser = BROWSER.CHROME;
    else if(browser.isFirefox)
      currentBrowser = BROWSER.FIREFOX;
    else if(browser.isOpera)
      currentBrowser = BROWSER.OPERA;
    else if(browser.isSafari)
      currentBrowser = BROWSER.SAFARI;
    else if(browser.isIe)
      currentBrowser = BROWSER.INTERNETEXPLORER;
    else
      currentBrowser = BROWSER.UNKNOWN;
  }
}

