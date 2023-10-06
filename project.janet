(declare-project :name "jw32")

(def debug-flags
  ["/DJW32_DEBUG"]
  #[]
  )
(def common-headers ["types.h" "debug.h"])

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :headers ["winuser.h" ;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib"])
 
(declare-native
 :name "jw32/processthreadsapi"
 :source ["processthreadsapi.c"]
 :headers ["processthreadsapi.h" ;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/libloaderapi"
 :source ["libloaderapi.c"]
 :headers ["libloaderapi.h" ;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/errhandlingapi"
 :source ["errhandlingapi.c"]
 :headers ["errhandlingapi.h" ;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/winbase"
 :source ["winbase.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags ["kernel32.lib"])

(declare-native
 :name "jw32/util"
 :source ["util.c"]
 :headers ["debug.h"]
 :cflags [;debug-flags]
 :ldflags [])
