import 'dart:convert';
import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';

const int GENERIC_READ = 0x80000000;
const int GENERIC_WRITE = 0x40000000;
const int OPEN_EXISTING = 3;
const int INVALID_HANDLE_VALUE = -1;
const int ERROR_PIPE_BUSY = 231;

typedef CreateFileW = Int32 Function(
  Pointer<Utf16> lpFileName,
  Uint32 dwDesiredAccess,
  Uint32 dwShareMode,
  Pointer lpSecurityAttributes,
  Uint32 dwCreationDisposition,
  Uint32 dwFlagsAndAttributes,
  Int32 hTemplateFile,
);

typedef CreateFileWDart = int Function(
  Pointer<Utf16> lpFileName,
  int dwDesiredAccess,
  int dwShareMode,
  Pointer lpSecurityAttributes,
  int dwCreationDisposition,
  int dwFlagsAndAttributes,
  int hTemplateFile,
);

typedef WriteFileNative = Int32 Function(
  Int32 hFile,
  Pointer<Uint8> lpBuffer,
  Uint32 nNumberOfBytesToWrite,
  Pointer<Uint32> lpNumberOfBytesWritten,
  Pointer lpOverlapped,
);

typedef WriteFileDart = int Function(
  int hFile,
  Pointer<Uint8> lpBuffer,
  int nNumberOfBytesToWrite,
  Pointer<Uint32> lpNumberOfBytesWritten,
  Pointer lpOverlapped,
);

typedef CloseHandleNative = Int32 Function(Int32 hObject);
typedef CloseHandleDart = int Function(int hObject);

typedef GetLastErrorNative = Uint32 Function();
typedef GetLastErrorDart = int Function();

class Pipe {
  final String name;
  const Pipe(this.name);

  static const String messageRunNormal = "0";
  static const String messageRunClassic = "1";
  static const String messageRunSettings = "2";

  static const String messageStop = "stop";
  static const String messageCloseAll = "closeAll";

  bool get isOpen {
    final kernel32 = DynamicLibrary.open('kernel32.dll');
    final createFile = kernel32.lookupFunction<CreateFileW, CreateFileWDart>('CreateFileW');
    final closeHandle = kernel32.lookupFunction<CloseHandleNative, CloseHandleDart>('CloseHandle');
    final getLastError = kernel32.lookupFunction<GetLastErrorNative, GetLastErrorDart>('GetLastError');
    final pipeNamePtr = name.toNativeUtf16();

    try {
      final handle = createFile(
        pipeNamePtr,
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        0,
      );

      if (handle != INVALID_HANDLE_VALUE) {
        closeHandle(handle);
        return true;
      }

      return getLastError() == ERROR_PIPE_BUSY;
    } finally {
      calloc.free(pipeNamePtr);
    }
  }

  bool sendMessage(String message) {
    final kernel32 = DynamicLibrary.open('kernel32.dll');
    final createFile = kernel32.lookupFunction<CreateFileW, CreateFileWDart>('CreateFileW');
    final closeHandle = kernel32.lookupFunction<CloseHandleNative, CloseHandleDart>('CloseHandle');
    final writeFile = kernel32.lookupFunction<WriteFileNative, WriteFileDart>('WriteFile');
    final pipeNamePtr = name.toNativeUtf16();

    if (kDebugMode) print("Sending message to pipe $name: $message");

    try {
      final handle = createFile(
        pipeNamePtr,
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        0,
      );

      if (handle == INVALID_HANDLE_VALUE) {
        closeHandle(handle);
        throw Exception("Invalid handle value: $handle (invalid: $INVALID_HANDLE_VALUE)");
      }

      final units = utf8.encode("$message|");
      final buffer = calloc<Uint8>(units.length);

      for (int i = 0; i < units.length; i++) {
        buffer[i] = units[i];
      }

      final bytesWritten = calloc<Uint32>();

      try {
        final success = writeFile(
          handle,
          buffer,
          units.length,
          bytesWritten,
          nullptr,
        );

        if (kDebugMode) print("Sent message to pipe $name: $message");
        return success != 0;
      } catch (e) {
        print("Error sending message to pipe $name: $e");
        return false;
      } finally {
        calloc.free(buffer);
        calloc.free(bytesWritten);
        closeHandle(handle);
      }
    } catch (e) {
      print("Error sending message to pipe $name: $e");
      return false;
    } finally {
      calloc.free(pipeNamePtr);
    }
  }
}