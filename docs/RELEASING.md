# 配布とGitHub Releases

## ビルド済みZIPの作成

ESP-IDF 5.5.5環境でルートから実行します。

```sh
idf.py build
python tools/package_release.py
python tests/test_release.py
```

`release/clock-25faces-non-c-16mb.zip` と `release/SHA256SUMS.txt` が生成されます。ZIP内には16MB用bootloader・partition table・通常アプリ、検証付き書き込みスクリプト、README、文字盤プレビュー、ライセンスが入ります。NVSダンプ、Wi-Fi認証情報、ローカルのシリアルログは含めません。SELF_TEST有効、16MB以外、コンパイル時認証情報のあるビルドは梱包を拒否します。

アプリ名 `thirteen_clock.bin` はプロジェクト名に由来する互換用の名前で、中身は25文字盤です。ZIPのSHA-256は配布物の破損確認用です。実機試験結果はdocs/VALIDATION.mdの測定済み版の記録であり、将来の任意のビルドを実機検証済みとするものではありません。

## GitHubに登録後

`.github/workflows/build.yml` はmainへのpush、Pull Request、手動実行でESP-IDF 5.5.5のビルドと配布ZIPの検証を行います。成功したActions実行のArtifactsからZIPを取得できます。

バージョンを決め、例えば次回の `v1.0.1` タグをpushすると、ビルド成功後にZIPとSHA256SUMSを添付した **Draft Release** を作成します。GitHubのReleasesで添付ファイルとリリース説明を確認し、Publish releaseで公開します。既に同じタグのReleaseがある場合は、その添付ファイルを保持して新規作成を省略します。

手動の場合はReleasesの「Draft a new release」で対象コミットのタグを選び、上記2ファイルを添付して、docs/RELEASE_NOTES.mdを説明に使用します。

新しいバージョンでは新しいタグを使います。公開済みタグの上書きや、以前のバイナリの差し替えは通常不要です。

## 初回Release

[v1.0.0](https://github.com/hiroikezawa/twenty-five-clock/releases/tag/v1.0.0) は、[GitHub Actionsでビルド・配布テストに成功したコミット](https://github.com/hiroikezawa/twenty-five-clock/actions/runs/38008322841) `38d65e3` の成果物です。ZIPのSHA-256は `ce34230b090d853642386d9b564335c9f260ed95e316c8eb8643edbceb4374aa`。Releaseへ添付後もサイズとハッシュの一致を確認しています。
