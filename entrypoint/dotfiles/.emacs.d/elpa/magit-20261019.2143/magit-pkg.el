;; -*- no-byte-compile: t; lexical-binding: nil -*-
(define-package "magit" "20261019.2143"
  "A Git porcelain inside Emacs."
  '((emacs         "28.1")
    (compat        "31.0")
    (cond-let      "1.1")
    (llama         "1.0")
    (magit-section "4.7")
    (seq           "2.24")
    (transient     "0.13")
    (with-editor   "3.5"))
  :url "https://github.com/magit/magit"
  :commit "d745ed24aa1c0b1fbc104a1d26ca609541ab9ed9"
  :revdesc "d745ed24aa1c"
  :keywords '("git" "tools" "vc")
  :authors '(("Marius Vollmer" . "marius.vollmer@gmail.com")
             ("Jonas Bernoulli" . "emacs.magit@jonas.bernoulli.dev"))
  :maintainers '(("Jonas Bernoulli" . "emacs.magit@jonas.bernoulli.dev")
                 ("Kyle Meyer" . "kyle@kyleam.com")))
