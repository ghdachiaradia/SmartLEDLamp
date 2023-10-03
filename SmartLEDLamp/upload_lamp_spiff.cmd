robocopy ../client data /E

"C:\Program Files\7-Zip\7z.exe" a -tgzip data\resources\bs.min.css.gz data\resources\bs.min.css
"C:\Program Files\7-Zip\7z.exe" a -tgzip data\resources\main.js.gz data\resources\main.js

DEL data\resources\bs.min.css
DEL data\resources\main.js
pause

C:\Users\dchiarad\.arduinocdt\packages\esp8266\tools\mkspiffs\0.2.0\mkspiffs.exe -p 256 -b 8192 -s 3125248 -c data spiffs-image.bin

python C:\Users\dchiarad\.arduinocdt\packages\esp8266\hardware\esp8266\2.3.0\tools\espota.py -i 192.168.178.22 -s -f "spiffs-image.bin"

rmdir /S /Q data
pause