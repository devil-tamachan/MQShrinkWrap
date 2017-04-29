# MQShrinkWrap
メタセコイア４用ShrinkWrap + Gravity互換モード。多角形対応

## ４種類の吸着モード
 - 通常のShrinkWrap
   - 一番近い面の一番近い面上へ頂点を移動
 - mqdlさんのGravity.dll互換モード
   - X軸のみ移動
   - Y軸のみ移動
   - Z軸のみ移動
   - カメラ方向のみ移動

## ダウンロード (64ビット版のみ）
 - https://github.com/devil-tamachan/MQShrinkWrap/releases
 - 32ビット版が欲しい人は自分でコンパイルしてください

## インストール方法
 - MQShrinkWrap.dll, CGAL-vc100-mt-4.9.1.dl_ の２ファイルを　Metasequoia4_x64\Plugins\Select　フォルダへコピー、貼り付け

## コンパイル方法
 - VCインストール
 - CGAL4.9.1をインストール (4.9.1までVC2010対応)
 - Boostインストール
 - ATLが無い場合は手に入れる。たしかPSDKかなんかで手に入るはず。手に入らない場合はA2Wを適当に書き換えてください
 - コンパイル
