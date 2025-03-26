(import spork/declare-cc)
(setdyn :verbose true)
(dofile "bundle/pm-project.janet" :env (declare-cc/jpm-shim-env))
