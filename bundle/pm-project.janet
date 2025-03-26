(import spork/path)
(import spork/cc)

#(import /script/util)
#(import /script/vcs)


(def PROJECT-NAME "jw32")

(def env (os/environ))
(def JANET-SOURCE-PATH
  (if-let [src-path (in env "JANET_SOURCE_PATH")]
    src-path
    (error "environment variable JANET_SOURCE_PATH not defined")))
# TODO: Read from env
(def DEBUG-DEFINES {"JW32_DEBUG" 1})
(def COMMON-HEADERS ["jw32.h" "types.h"])

# XXX: These are ripped from declare-cc.janet, should match
# the code there exactly.
(defn build-root [] (dyn *build-root* "_build"))
(defn build-dir [] (path/join (build-root) (dyn cc/*build-type* :release)))
(defn get-rules [] (dyn cc/*rules* (curenv)))

(defn add-deps [target & extra-deps]
  (if-let [inputs (get-in (get-rules) [target :inputs])]
    (array/push inputs ;extra-deps)
    (errorf "target %n not found" target)))
(defn out-path [name ext]
  (with-dyns [cc/*build-dir* (build-dir)]
    (cc/out-path name ext)))


(defn project-module [name]
  (string PROJECT-NAME "/" name))


(declare-project :name PROJECT-NAME)


(declare-native
 :name      (project-module "_winuser")
 :source    ["winuser.c"]
 :defines   DEBUG-DEFINES
 :msvc-libs ["user32.lib" "shcore.lib"]
 :c-std     "/TC")

(add-deps (out-path (project-module "_winuser") ".dll")
          "debug.h" ;COMMON-HEADERS)
