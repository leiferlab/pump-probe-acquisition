<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="12008004">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="NI.SortType" Type="Int">3</Property>
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="XImc Example One axis.vi" Type="VI" URL="../XImc Example One axis.vi"/>
		<Item Name="XImc Example Three axes.vi" Type="VI" URL="../XImc Example Three axes.vi"/>
		<Item Name="XImc simple example.vi" Type="VI" URL="../XImc simple example.vi"/>
		<Item Name="XImc example calb.vi" Type="VI" URL="../XImc example calb.vi"/>
		<Item Name="XImc simple example with network.vi" Type="VI" URL="../XImc simple example with network.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="Clear Errors.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Clear Errors.vi"/>
			</Item>
			<Item Name="user.lib" Type="Folder">
				<Item Name="init device.vi" Type="VI" URL="/&lt;userlib&gt;/libximc/VIs/init device.vi"/>
			</Item>
			<Item Name="XImc Example 1 CS.ctl" Type="VI" URL="../subvi/XImc Example 1 CS.ctl"/>
			<Item Name="Find Controllers 2.vi" Type="VI" URL="../subvi/Find Controllers 2.vi"/>
			<Item Name="libximc.lvlib" Type="Library" URL="../libximc.lvlib"/>
			<Item Name="Find Controllers.vi" Type="VI" URL="../subvi/Find Controllers.vi"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="XImc exe" Type="EXE">
				<Property Name="App_INI_aliasGUID" Type="Str">{976CAB6C-9B33-424B-AB07-D13FA6A49D19}</Property>
				<Property Name="App_INI_GUID" Type="Str">{F1743897-825E-475B-A1BF-0850C5935BD3}</Property>
				<Property Name="App_winsec.description" Type="Str">http://www.XI.com</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{899EAC7C-1A4E-4008-B745-187B45FDEDFE}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">XImc exe</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/XImc exe</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{E76B5C0A-EF36-41D3-9F60-6B98E577B705}</Property>
				<Property Name="Bld_targetDestDir" Type="Path"></Property>
				<Property Name="Destination[0].destName" Type="Str">ximcVieW.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/XImc exe/ximcVieW.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/XImc exe/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{5FBDAA63-563E-4692-987B-1AC088BBBE2B}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref"></Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_companyName" Type="Str">XI</Property>
				<Property Name="TgtF_fileDescription" Type="Str">XImc exe</Property>
				<Property Name="TgtF_fileVersion.major" Type="Int">1</Property>
				<Property Name="TgtF_internalName" Type="Str">XImc exe</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2011 XI</Property>
				<Property Name="TgtF_productName" Type="Str">XImc exe</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{930AC4FB-FAF8-4B78-B171-91941ADACF82}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">ximcVieW.exe</Property>
			</Item>
		</Item>
	</Item>
</Project>
