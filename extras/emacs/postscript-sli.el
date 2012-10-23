;; Major mode for editing SLI programs.
;;
;; Modified from postscript.el without permission form the author. Do not distribute.
;; Does also include some non-authorized ideas from idlwave.el.
;; Editors: M.-O. Gewaltig, R. Kupper
;;
;; The following two statements, placed in your init.el file or site-init.el,
;; will cause this file to be autoloaded, and PS/SLI-mode invoked, when
;; visiting .sli files:]
;;    (setq load-path (cons (concat (getenv "SLIHOME") "/extras/emacs") load-path)) 
;;    (load-library "sli")


;; NOTE:
;; This file generated from postscript.el, by replacing all occurences of "ps-" by "ps-sli",
;; "postscript" by "postscript-sli" (apart from "ps-postscript-command", which was replaced
;; by "ps-sli-sli-command").
;; Then some SLI-specific changes.



;; Header of the original postscript.el follows:

;; Major mode for editing postscript programs.
;;
;; Author:	Chris Maio
;; Last edit:	4 Sep 1988
;;
;; LCD Archive Entry:
;; postscript|Chris Maio|chris@cs.columbia.edu|
;; Major mode for editing PostScript programs.|
;; 4-Sep-1988||~/modes/postscript.el.Z|
;;
;; The following two statements, placed in your .emacs file or site-init.el,
;; will cause this file to be autoloaded, and postscript-mode invoked, when
;; visiting .ps or .cps files:
;;
;;	(autoload 'postscript-mode "postscript.el" "" t)
;;	(setq auto-mode-alist
;;	      (cons '("\\.c?ps$".postscript-mode) auto-mode-alist))
;;




(provide 'postscript-sli)
(require 'font-lock)
(require 'custom)


;; NOTE: all functions defined in this file start with "ps-sli-".

(defconst ps-sli-indent-level 2
  "*Indentation to be used inside of Postscript-Sli blocks or arrays")

(defconst ps-sli-tab-width 2
  "*Tab stop width for Postscript-Sli mode")

(defun ps-sli-make-tabs (stop)
  (and (< stop 132) (cons stop (ps-sli-make-tabs (+ stop ps-sli-tab-width)))))

(defconst ps-sli-tab-stop-list (ps-sli-make-tabs ps-sli-tab-width)
  "*Tab stop list for Postscript-Sli mode")


;;(defconst ps-sli-sli-command (list (concat (getenv "SLIHOME") "/nest/nest") "-")
;;  "*Command used to invoke a SLI shell.")
;;(defvar ps-sli-sli-command (list (concat (getenv "SLIHOME") "/nest/nest") "-")
;;  "*Command used to invoke a SLI shell.")

;; create the ps-sli-mode-map:
(defvar ps-sli-mode-map (make-sparse-keymap)
  "Keymap used in Postscript-SLI mode buffers")

(defvar ps-sli-mode-syntax-table nil
  "Postscript-Sli mode syntax table")

(defun postscript-sli-mode nil
  "Major mode for editing Postscript-Sli files.

\\[ps-sli-execute-buffer] will send the contents of the buffer to the NeWS
server using psh(1).  \\[ps-sli-execute-region] sends the current region.
\\[ps-sli-shell] starts an interactive psh(1) window which will be used for
subsequent \\[ps-sli-execute-buffer] or \\[ps-sli-execute-region] commands.

In this mode, TAB and \\[indent-region] attempt to indent code
based on the position of {}, [], and begin/end pairs.  The variable
ps-sli-indent-level controls the amount of indentation used inside
arrays and begin/end pairs.  

\\{ps-sli-mode-map}

\\[postscript-sli-mode] calls the value of the variable ps-sli-mode-hook with no args,
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map ps-sli-mode-map)
  (if ps-sli-mode-syntax-table
      (set-syntax-table ps-sli-mode-syntax-table)
      (progn
	(setq ps-sli-mode-syntax-table (make-syntax-table))
	(set-syntax-table ps-sli-mode-syntax-table)
	(modify-syntax-entry ?\( "<")
	(modify-syntax-entry ?\) ">")
	(modify-syntax-entry ?\[ "(\]")
	(modify-syntax-entry ?\] ")\[")
	(modify-syntax-entry ?\% "<")
	(modify-syntax-entry ?\n ">")))
  (make-local-variable 'comment-start)
  (make-local-variable 'comment-start-skip)
  (make-local-variable 'comment-column)
  (make-local-variable 'indent-line-function)
  (make-local-variable 'tab-stop-list)
  (setq comment-start "%"
	comment-start-skip "% *"
	comment-column 40
	indent-line-function 'ps-sli-indent-line
	tab-stop-list ps-sli-tab-stop-list)
  (setq mode-name "PS/SLI")
  (setq major-mode 'postscript-sli-mode)
  (run-hooks 'ps-sli-mode-hook))

(defun ps-sli-tab nil
  "Command assigned to the TAB key in Postscript-Sli mode."
  (interactive)
; This is a new version which does a fancy indentation from anywhere within
; a line. save-excursion saves the postion of the cursor
; Kupper & Gewaltig Nov. 2000
  (save-excursion 
    (ps-sli-indent-line)
    nil))

(defun ps-sli-indent-line nil
  "Indents a line of Postscript-Sli code."
  (interactive)
  (beginning-of-line)
  (delete-horizontal-space)
  (if (not (or (looking-at "%%")	; "%%" comments stay at left margin
	       (ps-sli-top-level-p)))
      (if (and (< (point) (point-max))
	       (eq ?\) (char-syntax (char-after (point)))))
	  (ps-sli-indent-close)		; indent close-delimiter
	(if (looking-at "end\\(using\\)?\\|cdef")
	    (ps-sli-indent-end)		; indent end token
	  (ps-sli-indent-in-block)))))	; indent line after open delimiter
  
(defun ps-sli-open nil
  (interactive)
  (insert last-command-char))

(defun ps-sli-insert-d-char (arg)
  "Awful hack to make \"end\" and \"cdef\" keywords indent themselves."
  (interactive "p")
  (insert-char last-command-char arg)
  (save-excursion
    (beginning-of-line)
    (if (looking-at "^[ \t]*\\(end\\(using\\)?\\|cdef\\)")
	(progn
	  (delete-horizontal-space)
	  (ps-sli-indent-end)))))

(defun ps-sli-close nil
  "Inserts and indents a close delimiter."
  (interactive)
  (insert last-command-char)
  (backward-char 1)
  (ps-sli-indent-close)
  (forward-char 1)
  (blink-matching-open))

(defun ps-sli-indent-close nil
  "Internal function to indent a line containing a an array close delimiter."
  (if (save-excursion (skip-chars-backward " \t") (bolp))
      (let (x (oldpoint (point)))
	(forward-char) (backward-sexp)	;XXX
	(if (and (eq 1 (count-lines (point) oldpoint))
		 (> 1 (- oldpoint (point))))
	    (goto-char oldpoint)
	  (beginning-of-line)
	  (skip-chars-forward " \t")
	  (setq x (current-column))
	  (goto-char oldpoint)
	  (delete-horizontal-space)
	  (indent-to x)))))

(defun ps-sli-indent-end nil
  "Indent an \"end\" token or array close delimiter."
  (let ((goal (ps-sli-block-start)))
    (if (not goal)
	(indent-relative)
      (setq goal (save-excursion
		   (goto-char goal) (back-to-indentation) (current-column)))
      (indent-to goal))))

(defun ps-sli-indent-in-block nil
  "Indent a line which does not open or close a block."
  (let ((goal (ps-sli-block-start)))
    (setq goal (save-excursion
		 (goto-char goal)
		 (back-to-indentation)
		 (if (bolp)
		     ps-sli-indent-level
		   (back-to-indentation)
		   (+ (current-column) ps-sli-indent-level))))
    (indent-to goal)))

;;; returns nil if at top-level, or char pos of beginning of current block

(defun ps-sli-block-start nil
  "Returns the character position of the character following the nearest
enclosing `[' `{' or `begin' keyword."
  (save-excursion
    (let (open (skip 0))
      (setq open (condition-case nil
		     (save-excursion
		       (backward-up-list 1)
		       (1+ (point)))
		   (error nil)))
      (ps-sli-begin-end-hack open))))

(defun ps-sli-begin-end-hack (start)
  "Search backwards from point to START for enclosing `begin' or 'namespace' and returns the
character number of the character following `begin' or START if not found.
Added search for 'namespace', Kupper, 21-jul-2003.
Added search for 'using/endusing', Kupper, 6-aug-2003."
;;Added search for 'namespace', Kupper, 21-jul-2003.
;;Added search for 'using/endusing', Kupper, 6-aug-2003.
  (save-excursion
    (let ((depth 1) match)
      (while (and (> depth 0)
		  (or (re-search-backward
		       "\\(\\(^[ \t]*\\(end\\(using\\)?\\)\\)\\|^[ \ta-z0-9\\/_-]*\\(begin\\|namespace\\|using\\)\\)[ \t]*\\(%.*\\)*$" start t)
		      (re-search-backward "^[ \t]*cdef.*$" start t)))
	(setq depth (if (looking-at "[ \t]*\\(end\\(using\\)?\\|end\\)")
			(1+ depth) (1- depth))))
      (if (not (eq 0 depth))
	  start
	(forward-word 1)
	(point)))))

(defun ps-sli-top-level-p nil
  "Awful test to see whether we are inside some sort of Postscript-Sli block."
  (and (condition-case nil
	   (not (scan-lists (point) -1 1))
	 (error t))
       (not (ps-sli-begin-end-hack nil))))

;;; initialize the keymap if it doesn't already exist
;(if (null ps-sli-mode-map)
;    (progn
;      (setq ps-sli-mode-map (make-sparse-keymap))
;; add to keymap:
      (define-key ps-sli-mode-map "d" 'ps-sli-insert-d-char)
      (define-key ps-sli-mode-map "f" 'ps-sli-insert-d-char)
      (define-key ps-sli-mode-map "{" 'ps-sli-open)
      (define-key ps-sli-mode-map "}" 'ps-sli-close)
      (define-key ps-sli-mode-map "[" 'ps-sli-open)
      (define-key ps-sli-mode-map "]" 'ps-sli-close)
      (define-key ps-sli-mode-map "\t" 'ps-sli-tab)
;;;      (define-key ps-sli-mode-map "\C-c\C-c" 'ps-sli-execute-buffer)
;;;      (define-key ps-sli-mode-map "\C-c|" 'ps-sli-execute-region)
;;;      (define-key ps-sli-mode-map "\C-c!" 'ps-sli-shell)
;      ))

;(defun ps-sli-execute-buffer nil
;  "Send contents of the buffer for execution to the SLI shell.
;If no SLI shell process is running, a SLI new shell is started."
;  (interactive)
;  (save-excursion
;    (mark-whole-buffer)
;    (ps-sli-execute-region (point-min) (point-max))))

;(defun ps-sli-execute-region (start end)
;  "Send the region between START and END for execution to the SLI shell.
;If no SLI shell process is running, a SLI new shell is started."
;  (interactive "r")
;  (let ((start (min (point) (mark)))
;	(end (max (point) (mark))))
;    (condition-case nil
;	(process-send-string "Postscript-Sli" (buffer-substring start end))
;      (error (shell-command-on-region start end ps-sli-sli-command nil)))))

;(defun ps-sli-shell nil
;  "Start a shell communicating with a Postscript-Sli printer or NeWS server."
;  (interactive)
;  (require 'shell)
;  (switch-to-buffer-other-window
;    (make-shell "Postscript-Sli" ps-sli-sli-command))
;  (make-local-variable 'shell-prompt-pattern)
;  (setq shell-prompt-pattern "^SLI .*\\] ")
;  (process-send-string "Postscript-Sli" "executive\n"))




 (defvar ps-sli-font-lock-keywords
    '(
      ("%.*$" . font-lock-comment-face)       ; PS Style comments
      ("\\/\\*.*$" . font-lock-comment-face)  ; This is a hack for c-style  comments
      ("^.*\\*\\/" . font-lock-comment-face)  ; it fontifies the lines containing /* and */
      ("\\([A-Za-z]\\)+:" . font-lock-doc-keyword-face) ; This fontifies documentation keywords
      ("\\<\\(def\\|for\\|repeat\\|loop\\|bind\\|SLIFunctionWrapper\\)\\>" . font-lock-keyword-face)
      ("\\<\\(if\\|ifelse\\|exit\\)\\>"               . font-lock-keyword-face)
      ("\\<\\(forall\\|forallindexed\\)\\>"           . font-lock-keyword-face)
      ("\\<\\(Map\\|MapIndexed\\|MapThread\\)\\>"  . font-lock-keyword-face)
      ("\\<\\(begin\\|end\\|namespace\\|using\\|endusing\\)\\>"       . font-lock-keyword-face)
      ("\\<\\(provide\\|provide-component\\|require\\|require-component\\)\\>" . font-lock-keyword-face)
      ("\\<\\(stop\\|stopped\\|raiseerror\\|raiseagain\\)\\>" . font-lock-keyword-face)
;;      ("[{}]" . font-lock-keyword-face)    ; fontify braces
      ("\\/\\([:.?A-Za-z0-9_]\\)+\\b" . font-lock-function-name-face) ; literal names
      ("([^()]*\\(([^()]*)\\)*[^()]*)" . font-lock-string-face)    ; hack for nested strings
     )
    "Default expressions to highlight in PS mode.")

;; Example how to add faces
;; First define a variable
(defvar font-lock-doc-keyword-face		'font-lock-doc-keyword-face
  "Face name to use for documentation keywords.")

;; then use it to define a new face.
(defface font-lock-doc-keyword-face
  '((((class grayscale) (background light)) (:foreground "LightGray" :bold t))
    (((class grayscale) (background dark))  (:foreground "DimGray" :bold t))
    (((class color) (background light))     (:foreground "Black" :underline t))
    (((class color) (background dark))      (:foreground "LightSteelBlue":underline t))
    (t (:bold t)))
  "Font Lock mode face used to highlight documentation keywords"
  :group 'font-lock-highlighting-faces)

(add-hook 'ps-sli-mode-hook
	  (function (lambda ()
		      (make-local-variable 'font-lock-defaults)
		      (setq font-lock-defaults '(ps-sli-font-lock-keywords t))
		      (font-lock-mode 1)
		      )))


