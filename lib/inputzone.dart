library InputZone;

import 'package:polymer/polymer.dart';
import 'dart:html';
import 'dart:js';
import 'dart:convert';

part 'FileTask.dart';

BROWSER currentBrowser = BROWSER.UNKNOWN;


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
  

  static get values => [NULL,START, CANCEL,PREPROGRESS,PROGRESS,ERROR,STATUS,OUTPUTURL,DETAILS];
}


class STATUSTYPE {
  final _value;
  const STATUSTYPE._internal(this._value);
  toString() => '$_value';

  static const CANCELED = const MESSAGETYPE._internal(0);
  static const STARTING = const MESSAGETYPE._internal(1);
  static const CONVERTING = const MESSAGETYPE._internal(2);
  static const COMPLETING = const MESSAGETYPE._internal(3);
  static const COMPLETED = const MESSAGETYPE._internal(4);
  static const ERRRORED = const MESSAGETYPE._internal(5);
  static const CANCELING = const MESSAGETYPE._internal(6);
  

  static get values => [CANCELED,STARTING, CONVERTING,COMPLETING,COMPLETED,ERRRORED,CANCELING];
}


class RUNTIMETYPE {
  final _value;
  const RUNTIMETYPE._internal(this._value);
  toString() => '$_value';

  static const EMSCR = const RUNTIMETYPE._internal(0);
  static const PNACL = const RUNTIMETYPE._internal(1);
  

  static get values => [EMSCR,PNACL];
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