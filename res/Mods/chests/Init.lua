function register_block_pair(_obj)
    _obj.places = _obj.places or _obj.id
    _obj.drops = _obj.drops or _obj.id
    voxel.block.register(_obj)
    voxel.item.register(_obj)
end

register_block_pair({
    name = "Chest",
    id = "chests.normal",
    textures = {
        "Assets/chest_normal_front.png", "Assets/chest_normal_side.png", "Assets/chest_normal_side.png",
        "Assets/chest_normal_side.png", "Assets/chest_normal_top.png", "Assets/chest_normal_bottom.png"
    },
    image = "Assets/chest_normal_front.png",
    rotH = true,
    onPlace = function(_pos)
			core.log_info("Placed a Chest!");
    end,
})

register_block_pair({
		name = "Iron Chest",
		id = "chests.iron", --Jack Co. Supply Crate ;3
		textures = {
				"Assets/chest_iron_front.png", "Assets/chest_iron_side.png", "Assets/chest_iron_side.png",
				"Assets/chest_iron_side.png", "Assets/chest_iron_top.png", "Assets/chest_iron_bottom.png"
		},
		image = "Assets/chest_iron_front.png",
		rotH = true,
		onBreak = function(_pos)
				core.log_info("CalledBreak")
		end,
		onPlace = function(_pos)
				--local block = voxel.map.getBlock(_pos)
				core.log_info("CalledPlace")
				--core.log_info(block['id'])
		end,
})
