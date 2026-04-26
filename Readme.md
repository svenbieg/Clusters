<h1>Clusters</h1>

<p>
I have written an algorithm, which can order and sort large numbers of items.<br />
It creates a pyramidal directory, moving entries and groups accordingly.<br />
Items can be added, removed and looked-up in constant low time.
</p><br />

<img src="https://github.com/user-attachments/assets/25a5f9b2-fbf7-4be2-85fd-642b8ce59aaf" width="400" /><br />
<br />

<p>
This is the standard-implementation, there are two more implementations in <a href="https://github.com/svenbieg/Heap">C</a> and <a href="https://github.com/svenbieg/Clusters.NET">C#</a>.
</p>
<br />

<h2>Principle</h2>
<br />

<table>
	<tr>
		<td><img src="https://github.com/user-attachments/assets/f072ba0a-f0b8-4bc0-97f3-9ced4df4e8fe" width="30" align="right" /></td>
		<td>The entries are stored in groups.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/bdd061b8-9c1b-4835-b80e-a97f24010145" width="30" align="right" /></td>
		<td>The size of the groups is limited and 10 by default.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/fec0cfa7-5a44-47c8-b3d8-f4cd2fdcdebd" width="60" align="right" /></td>
		<td>If the group is full a parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/e3d65374-7b7b-48f9-adff-c35a7cac015d" width="60" align="right" /></td>
		<td>The first and the last entry can be moved to the neighbour-group.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/ef7ca560-ee37-4bad-9a5e-30cec6bb3beb" width="60" align="right" /></td>
		<td>The entries are moved between the groups, so all groups get as full as possible.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/69a62898-36b2-498f-9020-0a825e1f3c8f" width="90" /></td>
		<td>The number of groups is limited too, another parent-group is created.</td>
	</tr><tr><td></td></tr><tr>
		<td><img src="https://github.com/user-attachments/assets/d3c32ae0-ad2e-4629-b5c8-bf4f54471bf6" width="90" /></td>
		<td>If an entry needs to be inserted in a full group, a whole sub-tree can be moved.</td>
	</tr>
</table><br />

<p>
You can find detailed information in the
<a href="https://github.com/svenbieg/Clusters/wiki/Home">Wiki</a>.
</p>
