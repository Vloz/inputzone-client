library InputZone;

import 'package:polymer/polymer.dart';
import 'iz_converter.dart';
import 'iz_app.dart';
import 'dart:html';
import 'dart:js';
import 'dart:convert';
import 'dart:async';

part 'FileTask.dart';

BROWSER currentBrowser = BROWSER.UNKNOWN;

IzApp iz_app;


class MESSAGETYPE {
  final _value;
  const MESSAGETYPE._internal(this._value);
  toString() => '$_value';

  static const NULL = const MESSAGETYPE._internal(0);
  static const START = const MESSAGETYPE._internal(1);
  static const CANCEL = const MESSAGETYPE._internal(2);
  static const PREPROGRESS = const MESSAGETYPE._internal(3);
  static const PROGRESS = const MESSAGETYPE._internal(4);
  static const ERROR = const MESSAGETYPE._internal(5);
  static const STATUS = const MESSAGETYPE._internal(6);
  static const OUTPUTURL = const MESSAGETYPE._internal(7);
  static const DETAILS = const MESSAGETYPE._internal(8);
  static const ESTIMATESIZE = const MESSAGETYPE._internal(9);
  

  static get values => [NULL,START, CANCEL,PREPROGRESS,PROGRESS,ERROR,STATUS,OUTPUTURL,DETAILS,ESTIMATESIZE];
}


class STATUSTYPE {
  final _value;
  const STATUSTYPE._internal(this._value);
  toString() => '$_value';

  static const CANCELED = const STATUSTYPE._internal(0);
  static const STARTING = const STATUSTYPE._internal(1);
  static const CONVERTING = const STATUSTYPE._internal(2);
  static const COMPLETING = const STATUSTYPE._internal(3);
  static const COMPLETED = const STATUSTYPE._internal(4);
  static const ERRRORED = const STATUSTYPE._internal(5);
  static const CANCELING = const STATUSTYPE._internal(6);
  static const OPTIMIZINGRAM = const STATUSTYPE._internal(7);
  static const WAITINGQUOTA = const STATUSTYPE._internal(8);
  static const WAITINGUSERCLICK = const STATUSTYPE._internal(9);
  static const ESTIMATINGOUTPUTSIZE = const STATUSTYPE._internal(10);
  

  static get values => [CANCELED,STARTING, CONVERTING,COMPLETING,COMPLETED,ERRRORED,CANCELING,OPTIMIZINGRAM,WAITINGQUOTA,WAITINGUSERCLICK,ESTIMATINGOUTPUTSIZE];
}


class RUNTIMETYPE {
  final _value;
  const RUNTIMETYPE._internal(this._value);
  toString() => '$_value';

  static const EMSCR = const RUNTIMETYPE._internal(0);
  static const PNACL = const RUNTIMETYPE._internal(1);
  

  static get values => [EMSCR,PNACL];
}

class FILESYSTEMTYPE {
  final _value;
  const FILESYSTEMTYPE._internal(this._value);
  toString() => '$_value';

  static const MEMFS = const FILESYSTEMTYPE._internal(0);
  static const HTML5TEMP = const FILESYSTEMTYPE._internal(1);
  static const HTML5PERS = const FILESYSTEMTYPE._internal(2);
  

  static get values => [MEMFS,HTML5TEMP,HTML5PERS];
}

class FS_PERS_STATUS {
  final _value;
  const FS_PERS_STATUS._internal(this._value);
  toString() => '$_value';

  static const CLOSED = const FS_PERS_STATUS._internal(0);
  static const OPENING = const FS_PERS_STATUS._internal(1);
  static const OPENED = const FS_PERS_STATUS._internal(2);
  

  static get values => [CLOSED,OPENING,OPENED];
}

class BROWSER {
  final _value;
  const BROWSER._internal(this._value);
  toString() => '$_value';

  static const UNKNOWN = const BROWSER._internal(0);
  static const CHROME = const BROWSER._internal(1);
  static const FIREFOX = const BROWSER._internal(2);
  static const OPERA = const BROWSER._internal(3);
  static const SAFARI = const BROWSER._internal(4);
  static const INTERNETEXPLORER = const BROWSER._internal(5);
  

  static get values => [UNKNOWN,CHROME,FIREFOX,OPERA,SAFARI,INTERNETEXPLORER];
}



class Html5FS {
  static FileSystem _persfs;
  static FileSystem _tempfs;
  
    static Future<FileSystem> pers(){
      if(_persfs==null)
      {
        var f = window.requestFileSystem(1024*1024*1024*50,  persistent: true);
        f.then((fs)=>_persfs=fs);
        return f;
      }
      else
        return new Future.value(_persfs);
    }
    
    static Future<FileSystem> temp(){
          if(_tempfs==null)
          {
            var f = window.requestFileSystem(1024*1024*1024*50,  persistent: false);
            f.then((fs)=>_tempfs=fs);
            return f;
          }
          else
            return new Future.value(_tempfs);
        }

}