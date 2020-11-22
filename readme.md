# autosample
autosampleは任意のMIDI機器に自動で最も低い音~最も高い音までのNOTE_ON, NOTE_OFFを送信し、  
その際に再生されたオーディオを録音して保存するツールです。  

## build
````
cmake . -DCMAKE_BUILD_TYPE=Release
make
make install
````

アンインストール時はバイナリを直接削除します。
````
which autosample
;rm -rf /usr/local/bin/autosample
````

## how to use
`autosample ls`と呼び出すと、使用可能なMIDI機器と使用可能なオーディオ機器の一覧が表示されます。
````
autosample ls
````

例えばこんな感じ
````
autosample ls
>MIDI OUT [0] IACドライバ IACバス1
>MIDI OUT [1] Steinberg UR22mkII  Port1
>AUDIO [0] Apple Inc.: Built-in Microphone
> InputChannels=2
> OutputChannels=0
> Format=[8bit signed int,16bit signed int,24bit signed int,32bit signed int,32bit >float,64bit float]
> SampleRate=[44100,48000]
> PreferredSampleRate=48000
>
>AUDIO [1] Apple Inc.: Built-in Output
> InputChannels=0
> OutputChannels=2
> Format=[8bit signed int,16bit signed int,24bit signed int,32bit signed int,32bit >float,64bit float]
> SampleRate=[44100,48000]
> PreferredSampleRate=48000
>
>AUDIO [2] ma++ ingalls for Cycling '74: Soundflower (2ch)
> InputChannels=2
> OutputChannels=2
> Format=[8bit signed int,16bit signed int,24bit signed int,32bit signed int,32bit >float,64bit float]
> SampleRate=[44100,48000,88200,96000,176400,192000]
> PreferredSampleRate=48000
>
>AUDIO [3] ma++ ingalls for Cycling '74: Soundflower (64ch)
> InputChannels=64
> OutputChannels=64
> Format=[8bit signed int,16bit signed int,24bit signed int,32bit signed int,32bit >float,64bit float]
> SampleRate=[44100,48000,88200,96000,176400,192000]
> PreferredSampleRate=48000
>
>AUDIO [4] Yamaha: Steinberg UR22mkII 
> InputChannels=2
> OutputChannels=2
> Format=[8bit signed int,16bit signed int,24bit signed int,32bit signed int,32bit >float,64bit float]
> SampleRate=[44100,48000,88200,96000,176400,192000]
> PreferredSampleRate=48000
````

`autosample rec`で録音が開始されます。  
この例では、`MIDI OUT [1] Steinberg UR22mkII  Port1`がMIDIイベントを受信して、その時`AUDIO [4] Yamaha: Steinberg UR22mkII`から発せられる音を録音しながら`AUDIO [1] Apple Inc.: Built-in Output`で録音時の音をモニタします。  
実際にはユーザごとに異なる環境を設定する必要があります。
````
autosample rec --midi 1 --audioIn 4 --audioOut 1 --low 60 --high 70
````

## 録音設定
サンプルレートは推奨サンプルレートが自動で使用されます。  
フォーマットは以下の順で検査していき、最初に使用可能なものが現れた時点でそれを使用する実装になっています。
* signed int 32
* signed int 24
* signed int 16
* signed int 8
* floating point 32
* floating point 64

## 使用可能なオプション
* --midi 使用するMIDI機器(デフォルト=0)
* --audioIn 使用する録音機器(デフォルト=0)
* --audioOut 録音時の音をモニタするための出力機器。通常は内蔵出力を指定してイヤホンなどで聴く(デフォルト=0)
* --low 最も低い音(デフォルト=0)
* --high 最も高い音(0~127, デフォルト=127)
* --milliseconds 再生時間(デフォルト=1000)
* --velocity 音の強さ(0~127, デフォルト=127)
* -n,--name 出力フォルダ名(デフォルトでは日付)

## その他
デフォルトでは`.raw`形式で保存されるので、`Sox`などのツールで必要に応じて変換してください。
````
sox -t raw -c 1 -r 8000 -b 16 -e signed-integer mono.raw -t wav mono.wav
````
> ※モノラル、サンプリング周波数8000Hz、量子化16ビットの非圧縮PCMデータをwavファイルにする例。

[音声ファイル形式をrawデータからwavに変換する方法](https://blog.goo.ne.jp/n-best/e/90fea78acf1eb039f291c0ee7a5fd76f)