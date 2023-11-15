(declare-project :name "jw32")

(def janet-src-tree "D:\\w\\janet_code\\janet")
(def cflags
  ["/nologo" "/W4"])
(def debug-flags
  [
   "/DJW32_DEBUG"
   #"/DJW32_CALLBACK_DEBUG"
  ]
  )
(def common-headers ["jw32.h" "types.h"])

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :headers ["debug.h" ;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["user32.lib"])
 
(declare-native
 :name "jw32/processthreadsapi"
 :source ["processthreadsapi.c"]
 :headers [;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["kernel32.lib"])

(declare-native
 :name "jw32/libloaderapi"
 :source ["libloaderapi.c"]
 :headers [;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["user32.lib" "kernel32.lib"])

(declare-native
 :name "jw32/errhandlingapi"
 :source ["errhandlingapi.c"]
 :headers [;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["user32.lib" "kernel32.lib"])

(declare-native
 :name "jw32/winbase"
 :source ["winbase.c"]
 :headers [;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["kernel32.lib"])
 
(declare-native
 :name "jw32/combaseapi"
 :source ["combaseapi.c"]
 :headers ["jw32_com.h" ;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags ["ole32.lib"])

(declare-native
 :name "jw32/uiautomation"
 :source ["uiautomation.c"]
 :headers ["jw32_com.h" "debug.h" ;common-headers]
 :cflags [# for accessing JanetVM internals defined in state.h from janet source tree
          (string "/I" janet-src-tree "\\src\\core")
          ;cflags
          ;debug-flags
          # disable warning C4200: nonstandard extension used: zero-sized array in struct/union
          # this warning is triggered by stuff in state.h, can't disable it with #pragma
          "/wd4200"]
 :ldflags ["oleaut32.lib"])

(declare-native
 :name "jw32/util"
 :source ["util.c"]
 :headers [;common-headers]
 :cflags [;cflags ;debug-flags]
 :ldflags [])
