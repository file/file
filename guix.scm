; SPDX-License-Identifier: PMPL-1.0-or-later
;; guix.scm — GNU Guix package definition for file-pr-fix
;; Usage: guix shell -f guix.scm

(use-modules (guix packages)
             (guix build-system gnu)
             (guix licenses))

(package
  (name "file-pr-fix")
  (version "0.1.0")
  (source #f)
  (build-system gnu-build-system)
  (synopsis "file-pr-fix")
  (description "file-pr-fix — part of the hyperpolymath ecosystem.")
  (home-page "https://github.com/hyperpolymath/file-pr-fix")
  (license ((@@ (guix licenses) license) "PMPL-1.0-or-later"
             "https://github.com/hyperpolymath/palimpsest-license")))
