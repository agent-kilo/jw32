(declare-project :name "jw32")

(def janet-src-tree "D:/w/janet_code/janet")
(def cflags
  [;(dyn :cflags) "/W4"])
(def ldflags
  [;(dyn :ldflags)])
(def debug-flags
  [
   "/DJW32_DEBUG"
   #"/DJW32_CALLBACK_DEBUG"
  ]
  )
(def common-headers ["jw32.h" "types.h"])


(def old-dynamic-cflags (dyn :dynamic-cflags))
(def old-dynamic-lflags (dyn :dynamic-lflags))

(defn project-module [name]
  (string ((dyn :project) :name) "/" name))

(with-dyns [:dynamic-cflags [;old-dynamic-cflags "/DJW32_DLL"]]
  (declare-native
   :name (project-module "winuser")
   :source ["winuser.c"]
   :headers ["debug.h" ;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "user32.lib"])
  
  (declare-native
   :name (project-module "processthreadsapi")
   :source ["processthreadsapi.c"]
   :headers [;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "kernel32.lib"])

  (declare-native
   :name (project-module "libloaderapi")
   :source ["libloaderapi.c"]
   :headers [;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "user32.lib" "kernel32.lib"])

  (declare-native
   :name (project-module "errhandlingapi")
   :source ["errhandlingapi.c"]
   :headers [;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "user32.lib" "kernel32.lib"])

  (declare-native
   :name (project-module "winbase")
   :source ["winbase.c"]
   :headers [;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "kernel32.lib"])
  
  (declare-native
   :name (project-module "combaseapi")
   :source ["combaseapi.c"]
   :headers ["jw32_com.h" ;common-headers]
   :cflags [;cflags ;debug-flags]
   :ldflags [;ldflags "ole32.lib"])

  (with-dyns [:dynamic-lflags [;old-dynamic-lflags
                               (string (find-build-dir) ((dyn :project) :name) "/" "combaseapi.lib")]]
    (declare-native
     :name (project-module "uiautomation")
     :source ["uiautomation.c"]
     :headers ["jw32_com.h" "debug.h" ;common-headers]
     :cflags [# for accessing JanetVM internals defined in state.h from janet source tree
              (string "/I" janet-src-tree "/src/core")
              ;cflags
              ;debug-flags
              # disable warning C4200: nonstandard extension used: zero-sized array in struct/union
              # this warning is triggered by stuff in state.h, can't disable it with #pragma
              "/wd4200"]
     :ldflags [;ldflags "oleaut32.lib"]))

  (add-dep (string (find-build-dir) ((dyn :project) :name) "/uiautomation.dll")
           (string (find-build-dir) ((dyn :project) :name) "/combaseapi.dll"))

  (declare-native
   :name (project-module "util")
   :source ["util.c"]
   :headers [;common-headers]
   :cflags [;cflags ;debug-flags]))
