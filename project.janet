(declare-project :name "jw32")

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :ldflags ["user32.lib"])
 
(declare-native
 :name "jw32/processthreadsapi"
 :source ["processthreadsapi.c"]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/libloaderapi"
 :source ["libloaderapi.c"]
 :ldflags ["user32.lib"])
