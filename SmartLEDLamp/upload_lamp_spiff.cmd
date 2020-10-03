robocopy data data_bin /E

"C:\Program Files\7-Zip\7z.exe" a -tgzip data_bin\resources\bs.min.css.gz data_bin\resources\bs.min.css
"C:\Program Files\7-Zip\7z.exe" a -tgzip data_bin\resources\bs.min.js.gz data_bin\resources\bs.min.js
"C:\Program Files\7-Zip\7z.exe" a -tgzip data_bin\resources\jq-3.1.1.min.js.gz data_bin\resources\jq-3.1.1.min.js
"C:\Program Files\7-Zip\7z.exe" a -tgzip data_bin\resources\pickr.min.js.gz data_bin\resources\pickr.min.js
"C:\Program Files\7-Zip\7z.exe" a -tgzip data_bin\resources\nano.min.css.gz data_bin\resources\nano.min.css

DEL data_bin\resources\bs.min.css
DEL data_bin\resources\bs.min.js
DEL data_bin\resources\jq-3.1.1.min.js
DEL data_bin\resources\pickr.min.js
DEL data_bin\resources\nano.min.css


C:\Users\dchiarad\.arduinocdt\packages\esp8266\tools\mkspiffs\0.2.0\mkspiffs.exe -p 256 -b 8192 -s 3125248 -c data_bin spiffs-image.bin

python C:\Users\dchiarad\.arduinocdt\packages\esp8266\hardware\esp8266\2.3.0\tools\espota.py -i 192.168.178.22 -s -f "spiffs-image.bin"

rmdir /S /Q data_bin
pause