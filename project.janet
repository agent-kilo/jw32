(declare-project :name "jw32")

(declare-native
 :name "jw32/winuser"
 :source ["winuser.c"]
 :ldflags ["user32.lib"])
