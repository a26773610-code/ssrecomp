POKEMON DS WIDESCREEN - XBOX UWP EXPERIMENTAL

Esta compilacao usa o frontend completo do projeto mamesj/pokemon-ds-widescreen-switch
(commit 5cdf42631a211abcef010f8b41ed542288f18a5f), adaptado para UWP x64.
Usa o interpretador do melonDS e renderizacao 3D por software.
NAO foi testada em um Xbox. FPS, imagem, audio e controles precisam ser verificados.
Nao e uma promessa de desempenho equivalente ao Switch. Nao inclui ROM.

1. No Xbox Dev Mode, instale PokemonDS-Widescreen-Xbox.appx pelo Device Portal.
   Se solicitado, instale a dependencia UWP Microsoft.VCLibs x64 e confie no
   certificado de teste fornecido. Configure o tipo do aplicativo como Game.
2. Execute o app uma vez; sem ROM ele deve mostrar instrucoes na tela.
3. No explorador de arquivos do Device Portal, abra LocalAppData, o pacote
   PokemonDS.Widescreen.Xbox, e sua pasta LocalState.
4. Coloque sua ROM compativel nessa pasta com o nome rom.nds. Reinicie o app.
   Jogos reconhecidos: SoulSilver/HeartGold US; Black/White/Black2/White2
   em ingles US/Europa; Platinum US. Outras revisoes/modificacoes nao verificadas.
5. Os saves ficam em LocalState: soulsilver.sav, heartgold.sav, black.sav,
   white.sav, black2.sav, white2.sav ou platinum.sav, conforme o jogo.
   game.log registra erros. Use sempre backup antes de importar um save.

O app abre em tela cheia com o compositor de uma tela do projeto original.
A/B/X/Y, direcional, gatilhos e analogicos passam pelo frontend original.
O menu e o cursor de toque seguem o mapeamento original; confira no Xbox.
Para trocar de jogo, feche o app e substitua rom.nds. Nao ha seletor de ROMs.

Codigo-fonte e instrucoes de build:
https://github.com/a26773610-code/ssrecomp/tree/xbox-widescreen-uwp/tools/uwp/widescreen
Original: https://github.com/mamesj/pokemon-ds-widescreen-switch
Licenca GPL-3.0-or-later. Sem afiliacao com Nintendo ou The Pokemon Company.
