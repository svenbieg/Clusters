<h1>Clusters</h1>

<p>
I have written an algorithm, wich can order and sort large numbers of items.<br />
It creates a pyramidal directory, moving entries and groups accordingly.<br />
Items can be added, removed and even kept sorted in real-time.
</p><br />

<a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256750-2e75a180-d485-11e8-8fe4-ad181f695690.jpg" style="" /></a><br />
<br />

<p>
This is the standard-implentation. Do also have a look at<br />
<a href="http://github.com/svenbieg/clusters-runtime">Clusters-Runtime</a>,
the Windows-Runtime-Component of Clusters!
</p>
<br />

<h2>Principle</h2>
<br />

<table>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256722-d3dc4580-d484-11e8-8393-b0e7c026be5e.png" /></a></td>
		<td>The entries are stored in groups.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256729-e48cbb80-d484-11e8-833e-846bb4a70b0c.png" /></a></td>
		<td>The size of the groups is limited and 10 by default.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256737-f4a49b00-d484-11e8-9171-a40ef63c3ff1.png" /></a></td>
		<td>If the group is full a parentgroup is created.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256739-ff5f3000-d484-11e8-9445-4443f52e228a.png" /></a></td>
		<td>The first and the last entry can be moved to the neighbour-group.</td>
	</tr><tr><td></td></tr><tr>
		<td><a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256742-09812e80-d485-11e8-8ca6-06a011e88120.png" /></a></td>
		<td>The entries are moved between the groups, so all groups get as full as possible.</td>
	</tr><tr><td></td></tr><tr>
		<td><a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256745-1736b400-d485-11e8-9785-e0479250b51d.png" /></a></td>
		<td>The number of groups is limited too, another parentgroup is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><a href="http://svenbieg.azurewebsites.net/Clusters"><img src="https://user-images.githubusercontent.com/12587394/47256748-21f14900-d485-11e8-9506-db75fa50c9bd.png" /></a></td>
		<td>If an entry needs to be inserted in a full group, a whole subtree can be moved.</td>
	</tr>
</table><br />

<p>
You can find detailed information on my
<a href="http://svenbieg.azurewebsites.net/clusters">Homepage</a>.
</p>

<br /><br /><br /><br /><br />
