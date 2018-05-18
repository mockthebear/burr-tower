--[[
	Part 1:
	Called from c++
]]


function onLoad()
	print("Hello from onLoad")
	local screen = g_screen.GetScreenSize()
	local ui = widgets.Window({
	--g_screen not working properly
		pos = {x=1,y=screen.y-80,},
		size = {x=screen.x-2,y=80},
		bgcolor = {r=255,g=158,b=94,a=255},


		update = function(this,dt)
			this:GetChildById("seeds"):SetText(string.format("Seeds: %.2d",  g_level.GetStorage(1)) )
			this:GetChildById("food"):SetText("Food: "..  g_level.GetStorage(4))

			this:GetChildById("shrum1"):SetEnable(g_level.GetStorage(1) >= 2)
			this:GetChildById("shrum2"):SetEnable(g_level.GetStorage(1) >= 4)
			this:GetChildById("shrum3"):SetEnable(g_level.GetStorage(1) >= 8)
			this:GetChildById("shrum4"):SetEnable(g_level.GetStorage(1) >= 16)

		end,




	})

	local sp = Sprite("data/torres.png")
	sp:SetClip(0,0,48,48)

	ui:AddComponent(widgets.SpriteLabel({
		id = "sh1",
		alignment = {
			top = "parent.top + 3",
			left = "barricade.right +80",
		},
		action =function(this)
			g_td.SetBuildMode(SELECTED_BUILD_TOWER1)
		end,

	},sp))


	ui:AddComponent(widgets.Button({
									str = "Build [1] Cost 2",
									action =function(this)
										g_level.SetBuildMode(1)
									end,
									id = "shrum1",
									alignment = {
										top = "sh1.bottom +3",
										vcenter = "sh1.center",
									},
	}))

	sp = Sprite("data/torres.png")
	sp:SetClip(48,0,48,48)
	ui:AddComponent(widgets.SpriteLabel({
		id = "sh2",
		alignment = {
			top = "parent.top + 3",
			left = "sh1.right +80",
		},
		action =function(this)
			g_td.SetBuildMode(SELECTED_BUILD_TOWER2)
		end,

	},sp))


	ui:AddComponent(widgets.Button({
									str = "Build [2] Cost 4",
									action =function(this)
										g_level.SetBuildMode(2)
									end,
									id = "shrum2",
									alignment = {
										top = "sh2.bottom +3",
										vcenter = "sh2.center",
									},
	}))

	sp = Sprite("data/torres.png")
	sp:SetClip(48 * 2,0,48,48)
	ui:AddComponent(widgets.SpriteLabel({
		id = "sh3",
		alignment = {
			top = "parent.top + 3",
			left = "sh2.right +80",
		},
		action =function(this)
			g_td.SetBuildMode(SELECTED_BUILD_TOWER3)
		end,

	},sp))


	ui:AddComponent(widgets.Button({
									str = "Build [2] Cost 8",
									action =function(this)
										g_level.SetBuildMode(3)
									end,
									id = "shrum3",
									alignment = {
										top = "sh3.bottom +3",
										vcenter = "sh3.center",
									},
	}))

	sp = Sprite("data/torres.png")
	sp:SetClip(48 * 3,0,48,48)
	ui:AddComponent(widgets.SpriteLabel({
		id = "sh4",
		action =function(this)
			g_td.SetBuildMode(SELECTED_BUILD_TOWER4)
		end,
		alignment = {
			top = "parent.top + 3",
			left = "sh3.right +80",
		},

	},sp))


	ui:AddComponent(widgets.Button({
									str = "Build [4] Cost 16",
									action =function(this)
										g_level.SetBuildMode(4)
									end,
									enabled = true,
									id = "shrum4",
									alignment = {
										top = "sh4.bottom +3",
										vcenter = "sh4.center",
									},
	}))



	ui:AddComponent(widgets.Button({
									str = "hp: ?",
									action =function(this)
										g_level.SetStorage(2,g_level.GetStorage(2)+5)
										this:SetText("hp  "..g_level.GetStorage(2))
									end,
									enabled = true,
									id = "butt",
									alignment = {
										bottom = "parent.bottom -3",
										vcenter = "parent.vcenter",
									},
	}))

	ui:AddComponent(widgets.Button({
									str = "speed: ?",
									action =function(this)
										g_level.SetStorage(3,g_level.GetStorage(3)+5)
										this:SetText("speed  "..g_level.GetStorage(3))

									end,
									enabled = true,
									id = "aaaa",
									alignment = {
										bottom = "butt.top -3",
										left = "butt.left",
									},
	}))

	ui:AddComponent(widgets.Button({
									str = "rst",
									action =function(this)
										g_level.SetStorage(3,1)
										g_level.SetStorage(2,1)

									end,
									enabled = true,
									id = "aaaae",
									alignment = {
										bottom = "aaaa.top -3",
										left = "butt.left",
									},
	}))

	ui.canMove = false
	ui:AddComponent(widgets.Label({
									str = "Seeds: ",
									id = "seeds",
									textsize = 36,
									alignment = {
										top = "parent.top + 10",
										right = "parent.right -32",
									},
	}) )
	ui:AddComponent(widgets.Label({
									str = "Food: ",
									id = "food",
									textsize = 26,
									alignment = {
										top = "seeds.bottom + 4",
										right = "parent.right -32",
									},
	}) )

	g_ui.AddWindow(ui)

end

