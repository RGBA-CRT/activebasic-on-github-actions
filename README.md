# activebasic-on-github-actions
github actionsを使ってActiveBasicアプリをビルドする実験

## badge test
![Build status](https://github.com/RGBA-CRT/activebasic-on-github-actions/actions/workflows/activebasic.yml/badge.svg)

# 結果：できない
- runnerがself hostではBasicCompilerを動かしてEXEができる
- runnerがwindows-2019だとBasicCompilerが動かない
	- stracentで見てもすぐ落ちてる
	- ProjectEditorをPSから動かしてビルドしたがやはりBasicCompilerがすぐに落ちているらしい
	- windows2019をAzureで動かしてみようとしたがEXEのコピーでエラーで動かなかった