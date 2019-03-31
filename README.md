EmuRPC - Framework for integrating external scripting support geared towards emulators
===

## 

Emulators often get requests to add a scripting layer to the emulator, but there isn't really any good solution to this problem yet. Adding a embedded language to the emulator means that you are now locked to that language and implementation, limiting platform availability of the emulator, and restricting users to a single language.

EmuRPC makes it simple for emu devs to add a powerful and flexible RPC to their emulator with minimal effort. This

## 



## License

EmuRPC is under the MIT license, but links against two libraries that are not MIT.

TinyCThread (Optional) - zlib/libpng license
libwebsockets - LGPLv2 with static linking exception.

EmuRPC defaults to static linking libwebsockets, but because of the static linking exception, there is no requirement for users of this library to make their source available or required to allow users to replace the lib with their own copy. As such, this project should be fine to use in closed source applications or those that are not generally able to use LGPLv2 license projects.

If relying on a LGPL library proves to be a constant concern for implementors, then we can consider adding support for other libraries or rolling an in house minimal websocket handler on libuv.