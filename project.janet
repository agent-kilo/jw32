(declare-project :name "jw32")

(def env (os/environ))

(def janet-src-tree
  (if-let [src-path (in env "JANET_SOURCE_PATH")]
    src-path
    (error "environment variable JANET_SOURCE_PATH not defined")))

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

(with-dyns [:cflags [;cflags ;debug-flags]
            :dynamic-cflags [;old-dynamic-cflags "/DJW32_DLL"]]
  (declare-native
   :name (project-module "_winuser")
   :source ["winuser.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "user32.lib" "shcore.lib"])

  (declare-native
   :name (project-module "_processthreadsapi")
   :source ["processthreadsapi.c"]
   :headers [;common-headers]
   :ldflags [;ldflags "kernel32.lib" "advapi32.lib"])

  (declare-native
   :name (project-module "_libloaderapi")
   :source ["libloaderapi.c"]
   :headers [;common-headers]
   :ldflags [;ldflags "user32.lib" "kernel32.lib"])

  (declare-native
   :name (project-module "_errhandlingapi")
   :source ["errhandlingapi.c"]
   :headers [;common-headers]
   :ldflags [;ldflags "user32.lib" "kernel32.lib"])

  (declare-native
   :name (project-module "_winbase")
   :source ["winbase.c"]
   :headers [;common-headers]
   :ldflags [;ldflags "kernel32.lib"])
  
  (declare-native
   :name (project-module "_combaseapi")
   :source ["combaseapi.c"]
   :headers ["jw32_com.h" ;common-headers]
   :ldflags [;ldflags "ole32.lib"])

  (with-dyns [:dynamic-lflags [;old-dynamic-lflags
                               (string (find-build-dir) ((dyn :project) :name) "/" "_combaseapi.lib")]]
    (declare-native
     :name (project-module "_uiautomation")
     :source ["uiautomation.c"]
     :headers ["jw32_com.h" "debug.h" ;common-headers]
     :cflags [;(dyn :cflags)
              # for accessing JanetVM internals defined in state.h from janet source tree
              (string "/I" janet-src-tree "/src/core")
              # disable warning C4200: nonstandard extension used: zero-sized array in struct/union
              # this warning is triggered by stuff in state.h, can't disable it with #pragma
              "/wd4200"]
     :ldflags [;ldflags "ole32.lib" "oleaut32.lib"])

    (declare-native
     :name (project-module "_oaidl")
     :source ["oaidl.c"]
     :headers ["jw32_com.h" "debug.h" ;common-headers]
     :ldflags [;ldflags])

    (declare-native
     :name (project-module "_shobjidl_core")
     :source ["shobjidl_core.c"]
     :headers ["jw32_com.h" "debug.h" ;common-headers]
     :ldflags [;ldflags "ole32.lib" "oleaut32.lib"]))

  (add-dep (string (find-build-dir) ((dyn :project) :name) "/_uiautomation.dll")
           (string (find-build-dir) ((dyn :project) :name) "/_combaseapi.dll"))

  (add-dep (string (find-build-dir) ((dyn :project) :name) "/_oaidl.dll")
           (string (find-build-dir) ((dyn :project) :name) "/_combaseapi.dll"))

  (add-dep (string (find-build-dir) ((dyn :project) :name) "/_shobjidl_core.dll")
           (string (find-build-dir) ((dyn :project) :name) "/_combaseapi.dll"))

  (declare-native
   :name (project-module "_shellapi")
   :source ["shellapi.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "shell32.lib"])

  (with-dyns [:dynamic-lflags [;old-dynamic-lflags
                               "/manifest:embed"
                               "/manifestinput:manifest/commctrl.manifest"]]
    (declare-native
     :name (project-module "_commctrl")
     :source ["commctrl.c"]
     :headers ["debug.h" ;common-headers]
     :ldflags [;ldflags "comctl32.lib"]))

  (declare-native
   :name (project-module "_consoleapi")
   :source ["consoleapi.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "kernel32.lib"])

  (declare-native
   :name (project-module "_securitybaseapi")
   :source ["securitybaseapi.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "advapi32.lib"])

  (declare-native
   :name (project-module "_winnt")
   :source ["winnt.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags])

  (declare-native
   :name (project-module "_handleapi")
   :source ["handleapi.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "kernel32.lib"])

  (declare-native
   :name (project-module "_dwmapi")
   :source ["dwmapi.c"]
   :headers ["debug.h" ;common-headers]
   :ldflags [;ldflags "dwmapi.lib"])

  (declare-native
   :name (project-module "_util")
   :source ["util.c"]
   :headers [;common-headers]))
