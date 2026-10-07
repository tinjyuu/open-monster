# モンスターデータ v1

[English](../monster-format.md) · [日本語](monster-format.md)

通常のモンスター追加は、データとドット絵・翻訳だけで行えます。名前・生態は表示文章ではなく、`monsters.sample.name` のような翻訳キーを参照します。

| 項目 | 内容 |
| --- | --- |
| id | 一意の英小文字・数字・`_`・`-`。英小文字で開始 |
| save_id | 永続整数ID。0〜254、重複禁止。初期0〜5は予約済みで変更しない |
| name / bio | 両言語とcontextに存在する翻訳キー。名前8セル、生態18セル |
| starter | 追加モンスターはfalse。初期3種のみtrue |
| stats | 整数。HP24〜40、攻撃・防御・速さ5〜13、合計70以下 |
| base_moves | 攻撃系の技、guard、dodgeの順に3つ |
| training | attack・defense・techniqueごとに既存技IDを指定 |
| trait | steam・rain・warm・shell・spore・quick |
| palette | 4色の`#RRGGBB`。背景/透明・輪郭・体色・明色 |
| pixels | 16×16または32×32。`.`背景、`1`輪郭、`2`体色、`3`明色 |
| authors | 投稿者・共同作者名の配列 |

## 追加の例

```sh
python3 scripts/new_monster.py sample \
  --name-en SAMPLE --name-ja サンプル \
  --bio-en 'A TRAVELING TEAPOT' --bio-ja 'タビヲ スル キュウス' \
  --author 'Your name'
make test && make
```

補助ツールは未使用ID、初期データ、両言語とcontextの翻訳キーを作ります。元はチャポの例なので、作品として提案する場合は設定・絵・戦い方を自作してください。新しい種は自動で野生の出現候補に入ります。

戦闘用は32×32。従来の16×16は整数倍で拡大し、32×32の絵はそのまま使います。探索用は16×16に間引くので、両方のサイズを確認してください。色はGBCの5bit RGBへ変換します。

## 技・特性と互換性

既存12技の順序はセーブIDに対応するため変更禁止です。新しい技や特性は別の設計提案から始めます。特性はsteam=貫通+2、rain=コサメ+2、warm=回復+3、shell=被ダメージ-1、spore=吸収回復+2、quick=行動速度+3です。

訓練で対応する能力+2と技を獲得し、変更しても習得済みの技は残ります。拠点で3つの技を装備できます。

現在のデータ検査上限は24種で、実際のROM容量にも制約されます。通常の追加にはゲーム本体の変更は不要ですが、既存IDの変更・削除には移行設計が必要です。クレジットの完全版はJSONとCREDITSに記載します。
