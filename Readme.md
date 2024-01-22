<h1>Clusters</h1>

<p>
I have written an algorithm, which can order and sort large numbers of items.<br />
It creates a pyramidal directory, moving entries and groups accordingly.<br />
Items can be added, removed and looked-up in constant low time.
</p><br />

<img src="https://user-images.githubusercontent.com/12587394/47256750-2e75a180-d485-11e8-8fe4-ad181f695690.jpg" style="" /><br />
<br />
<br />

<p>
I'm using it in my <a href="https://github.com/svenbieg/Heap">memory-manager</a>.
</p>
<br />

<h2>Principle</h2>
<br />

<table>
	<tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="https://user-images.githubusercontent.com/12587394/47256722-d3dc4580-d484-11e8-8393-b0e7c026be5e.png" /></td>
		<td>The entries are stored in groups.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="https://user-images.githubusercontent.com/12587394/47256729-e48cbb80-d484-11e8-833e-846bb4a70b0c.png" /></td>
		<td>The size of the groups is limited and 10 by default.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="https://user-images.githubusercontent.com/12587394/47256737-f4a49b00-d484-11e8-9171-a40ef63c3ff1.png" /></td>
		<td>If the group is full a parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;<img src="https://user-images.githubusercontent.com/12587394/47256739-ff5f3000-d484-11e8-9445-4443f52e228a.png" /></td>
		<td>The first and the last entry can be moved to the neighbour-group.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://user-images.githubusercontent.com/12587394/47256742-09812e80-d485-11e8-8ca6-06a011e88120.png" /></td>
		<td>The entries are moved between the groups, so all groups get as full as possible.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://user-images.githubusercontent.com/12587394/47256745-1736b400-d485-11e8-9785-e0479250b51d.png" /></td>
		<td>The number of groups is limited too, another parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://user-images.githubusercontent.com/12587394/47256748-21f14900-d485-11e8-9506-db75fa50c9bd.png" /></td>
		<td>If an entry needs to be inserted in a full group, a whole sub-tree can be moved.</td>
	</tr>
</table><br />

<p>
You can find detailed information in the
<a href="https://github.com/svenbieg/Clusters/wiki/Home">Wiki</a>.
</p><br />

Best regards,<br />
<br />
Sven Bieg<br />

<br /><br /><br /><br /><br />
