(define BUFSIZE 512)

(define getBatteryTime (foreign-lambda c-string "getBatteryTime" c-string))
(print (getBatteryTime (make-string BUFSIZE)))

;(print
;  ((foreign-lambda c-string "getBatteryTime" c-string)
;   (make-string BUFSIZE)))
