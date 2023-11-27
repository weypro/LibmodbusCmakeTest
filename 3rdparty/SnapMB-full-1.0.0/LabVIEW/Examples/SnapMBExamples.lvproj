<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="20008000">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Client" Type="Folder">
			<Item Name="ClientDemo.vi" Type="VI" URL="../Client/ClientDemo.vi"/>
			<Item Name="ClientSerial.vi" Type="VI" URL="../Client/ClientSerial.vi"/>
		</Item>
		<Item Name="Controller" Type="Folder">
			<Item Name="FieldControllerDemo.vi" Type="VI" URL="../FieldController/FieldControllerDemo.vi"/>
		</Item>
		<Item Name="Device" Type="Folder">
			<Item Name="DeviceDemo.vi" Type="VI" URL="../Device/DeviceDemo.vi"/>
		</Item>
		<Item Name="SnapMB.lvlib" Type="Library" URL="../../lib/SnapMB.lvlib"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="snapmb.dll" Type="Document" URL="../../lib/windows/snapmb.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="CliSerial" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{7F477959-C2DA-4CC3-8D72-8C0AE93F4E1E}</Property>
				<Property Name="App_INI_GUID" Type="Str">{8C25D968-5CDB-419E-8E89-6BE3359D9B46}</Property>
				<Property Name="App_serverConfig.httpPort" Type="Int">8002</Property>
				<Property Name="App_serverType" Type="Int">0</Property>
				<Property Name="Bld_autoIncrement" Type="Bool">true</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{46D31792-411E-4E3F-8D76-C6031CCC96F0}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">CliSerial</Property>
				<Property Name="Bld_excludeInlineSubVIs" Type="Bool">true</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/CliSerial</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{D237541C-2274-489D-BDB1-4B0F02574B23}</Property>
				<Property Name="Bld_version.build" Type="Int">1</Property>
				<Property Name="Bld_version.major" Type="Int">1</Property>
				<Property Name="Destination[0].destName" Type="Str">Application.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/CliSerial/Application.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/CliSerial/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{BCCA7C85-D932-45C1-9A58-847B0F6EC9B5}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref"></Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_fileDescription" Type="Str">CliSerial</Property>
				<Property Name="TgtF_internalName" Type="Str">CliSerial</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2022 </Property>
				<Property Name="TgtF_productName" Type="Str">CliSerial</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{B90AEF35-B382-4B49-A5F7-CD24952E665A}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">Application.exe</Property>
				<Property Name="TgtF_versionIndependent" Type="Bool">true</Property>
			</Item>
		</Item>
	</Item>
</Project>
