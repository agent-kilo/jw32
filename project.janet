(declare-project :name "jw32")

(def debug-flags
  ["/DJW32_DEBUG"]
  #[]
  )
(def common-headers ["jw32.h" "types.h"])

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :headers ["debug.h" ;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib"])
 
(declare-native
 :name "jw32/processthreadsapi"
 :source ["processthreadsapi.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags ["kernel32.lib"])

(declare-native
 :name "jw32/libloaderapi"
 :source ["libloaderapi.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib" "kernel32.lib"])

(declare-native
 :name "jw32/errhandlingapi"
 :source ["errhandlingapi.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags ["user32.lib" "kernel32.lib"])

(declare-native
 :name "jw32/winbase"
 :source ["winbase.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags ["kernel32.lib"])

(declare-native
 :name "jw32/util"
 :source ["util.c"]
 :headers [;common-headers]
 :cflags [;debug-flags]
 :ldflags [])
