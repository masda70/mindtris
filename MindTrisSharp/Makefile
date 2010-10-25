all: MindTrisSharpServer.exe MindTrisSharp.exe

MindTrisSharpServer.exe: ./MindTrisCore/*.cs ./MindTrisServer/*.cs
	gmcs -out:MindTrisSharpServer.exe ./MindTrisCore/*.cs ./MindTrisServer/*.cs

MindTrisSharp.exe: ./MindTrisCore/*.cs ./MindTris/*.cs
	gmcs -out:MindTrisSharp.exe ./MindTrisCore/*.cs ./MindTris/*.cs