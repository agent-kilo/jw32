(declare-project :name "jw32")

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :headers ["winuser.h" "types.h"]
 :ldflags ["user32.lib"])
 
(declare-native
 :name "jw32/processthreadsapi"
 :source ["processthreadsapi.c"]
 :headers ["processthreadsapi.h" "types.h"]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/libloaderapi"
 :source ["libloaderapi.c"]
 :headers ["libloaderapi.h" "types.h"]
 :ldflags ["user32.lib"])

(declare-native
 :name "jw32/errhandlingapi"
 :source ["errhandlingapi.c"]
 :headers ["errhandlingapi.h" "types.h"]
 :ldflags ["user32.lib"])
