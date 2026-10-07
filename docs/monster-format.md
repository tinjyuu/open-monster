# モンスターデータ v1

JSONひとつで通常のモンスターを追加できます。元の16×16ドット絵をバトルでは整数倍して32×32、探索では16×16として使います。

| 項目 | 内容 |
| --- | --- |
| id | 一意の英小文字・数字・`_`・`-`。英小文字で開始 |
| save_id | 永続ID。0〜254、重複禁止。既存IDは変更しない |
| name / bio | 名前は8文字以下、生態は18文字以下。独自フォントにあるカタカナ・英大文字・数字・空白を使用 |
| starter | 追加モンスターはfalse。初期3種のみtrue |
| stats | hp:24〜40、attack/defense/speed:5〜13、合計70以下 |
| base_moves | 3つ。攻撃系の技、guard、dodgeの順 |
| training | attack、defense、techniqueごとに既存技IDを指定 |
| trait | steam、rain、warm、shell、spore、quickのいずれか |
| palette | 4色の`#RRGGBB`。0背景/透明、1輪郭、2体色、3明色 |
| pixels | 16行×16文字。`.`=背景/透明、`1`=輪郭、`2`=体色、`3`=明色 |
| authors | 投稿者・共同作者名の配列 |

## 動く追加の例

```sh
python3 - <<'PY'
import json
from pathlib import Path
p=Path('data/monsters')
m=json.loads((p/'chapo.json').read_text())
m.update(id='sample',save_id=6,name='サンプル',bio='タビヲ スル キュウス',starter=False,authors=['Your name'])
(p/'sample.json').write_text(json.dumps(m,ensure_ascii=False,indent=2)+'\n')
PY
make test
make
```

これはチャポの元データを再利用する技術的な例です。作品として提案する場合は、その子の設定と絵、戦い方を作ってください。

## 技と特性

技IDは `data/moves.json` を参照。順序はv1セーブの技IDに対応しているため変更禁止です。通常の追加では既存12技を選びます。

特性の効果：steamは貫通技+2、rainはコサメ+2、warmは回復技+3、shellは被ダメージ-1、sporeは吸収回復+2、quickは行動速度+3です。訓練は習得技と該当能力+2を与え、方針を変更しても習得技は残ります。

`make` はドット絵を2bppのタイル、パレットをGBC形式、設定をCのテーブルに変換します。JSONはゲーム内で直接解析しません。使用できない文字、異常な能力、重複ID、壊れた絵はビルドで拒否します。作者クレジットの完全版はJSONとCREDITSに残し、ゲーム内ではGitHubの案内を表示します。
